/*! \file
 * \brief contains a PNG library
 *
 * The PNG library 'uPNG' was written by Sean Middleditch and Lode Vandevenne
 * and modified for StuBS by Bernhard Heinloth.
 *
 * \see https://github.com/elanthis/upng
 *
 * uPNG -- derived from LodePNG version 20100808
 *
 * Copyright (c) 2005-2010 Lode Vandevenne
 * Copyright (c) 2010 Sean Middleditch
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *		1. The origin of this software must not be misrepresented; you must not
 *		claim that you wrote the original software. If you use this software
 *		in a product, an acknowledgment in the product documentation would be
 *		appreciated but is not required.
 *
 *		2. Altered source versions must be plainly marked as such, and must not be
 *		misrepresented as being the original software.
 *
 *		3. This notice may not be removed or altered from any source
 *		distribution.
 */
#include "utils/png.h"
#include "utils/alloc.h"
#include "utils/string.h"
#include "fs/vfs.h"

#define MAKE_BYTE(b) ((b) & 0xFF)
#define MAKE_DWORD(a, b, c, d) ((MAKE_BYTE(a) << 24) | (MAKE_BYTE(b) << 16) | (MAKE_BYTE(c) << 8) | MAKE_BYTE(d))
#define MAKE_DWORD_PTR(p) MAKE_DWORD((p)[0], (p)[1], (p)[2], (p)[3])

#define CHUNK_IHDR MAKE_DWORD('I', 'H', 'D', 'R')
#define CHUNK_IDAT MAKE_DWORD('I', 'D', 'A', 'T')
#define CHUNK_IEND MAKE_DWORD('I', 'E', 'N', 'D')

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285

#define NUM_DEFLATE_CODE_SYMBOLS 288  // 256 literals, the end code, some length codes, and 2 unused codes
#define NUM_DISTANCE_SYMBOLS 32       // the distance codes have their own symbols, 30 used, 2 unused
#define NUM_CODE_LENGTH_CODES 19      // the code length codes.
                                      //    0-15: code lengths,
                                      //      16: copy previous 3-6 times,
                                      //      17: 3-10 zeros,
                                      //      18: 11-138 zeros
#define MAX_SYMBOLS 288               // largest number of symbols used by any tree type

#define DEFLATE_CODE_BITLEN 15
#define DISTANCE_BITLEN 15
#define CODE_LENGTH_BITLEN 7
#define MAX_BIT_LENGTH 15  // largest bitlen used by any tree type

#define DEFLATE_CODE_BUFFER_SIZE (NUM_DEFLATE_CODE_SYMBOLS * 2)
#define DISTANCE_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)
#define CODE_LENGTH_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)

#define SET_ERROR(code) do { error = (code); error_line = __LINE__; } while (0)

#define chunk_length(chunk) MAKE_DWORD_PTR(chunk)
#define chunk_type(chunk) MAKE_DWORD_PTR((chunk) + 4)
#define chunk_critical(chunk) (((chunk)[4] & 32) == 0)

// the base lengths represented by codes 257-285
static const unsigned LENGTH_BASE[29] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
	67, 83, 99, 115, 131, 163, 195, 227, 258
};

// the extra bits used by codes 257-285 (added to base length)
static const unsigned LENGTH_EXTRA[29] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5,
	5, 5, 5, 0
};

// the base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree)
static const unsigned DISTANCE_BASE[30] = {
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
	769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

// the extra bits of backwards distances (added to base)
static const unsigned DISTANCE_EXTRA[30] = {
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
	11, 11, 12, 12, 13, 13
};

// the order in which "code length alphabet code lengths" are stored,
// out of this the huffman tree of the dynamic huffman tree lengths is generated
static const unsigned CLCL[NUM_CODE_LENGTH_CODES] = {
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static unsigned FIXED_DEFLATE_CODE_TREE[NUM_DEFLATE_CODE_SYMBOLS * 2] = {
	289, 370, 290, 307, 546, 291, 561, 292, 293, 300, 294, 297, 295, 296, 0, 1,
	2, 3, 298, 299, 4, 5, 6, 7, 301, 304, 302, 303, 8, 9, 10, 11, 305, 306, 12,
	13, 14, 15, 308, 339, 309, 324, 310, 317, 311, 314, 312, 313, 16, 17, 18,
	19, 315, 316, 20, 21, 22, 23, 318, 321, 319, 320, 24, 25, 26, 27, 322, 323,
	28, 29, 30, 31, 325, 332, 326, 329, 327, 328, 32, 33, 34, 35, 330, 331, 36,
	37, 38, 39, 333, 336, 334, 335, 40, 41, 42, 43, 337, 338, 44, 45, 46, 47,
	340, 355, 341, 348, 342, 345, 343, 344, 48, 49, 50, 51, 346, 347, 52, 53,
	54, 55, 349, 352, 350, 351, 56, 57, 58, 59, 353, 354, 60, 61, 62, 63, 356,
	363, 357, 360, 358, 359, 64, 65, 66, 67, 361, 362, 68, 69, 70, 71, 364,
	367, 365, 366, 72, 73, 74, 75, 368, 369, 76, 77, 78, 79, 371, 434, 372,
	403, 373, 388, 374, 381, 375, 378, 376, 377, 80, 81, 82, 83, 379, 380, 84,
	85, 86, 87, 382, 385, 383, 384, 88, 89, 90, 91, 386, 387, 92, 93, 94, 95,
	389, 396, 390, 393, 391, 392, 96, 97, 98, 99, 394, 395, 100, 101, 102, 103,
	397, 400, 398, 399, 104, 105, 106, 107, 401, 402, 108, 109, 110, 111, 404,
	419, 405, 412, 406, 409, 407, 408, 112, 113, 114, 115, 410, 411, 116, 117,
	118, 119, 413, 416, 414, 415, 120, 121, 122, 123, 417, 418, 124, 125, 126,
	127, 420, 427, 421, 424, 422, 423, 128, 129, 130, 131, 425, 426, 132, 133,
	134, 135, 428, 431, 429, 430, 136, 137, 138, 139, 432, 433, 140, 141, 142,
	143, 435, 483, 436, 452, 568, 437, 438, 445, 439, 442, 440, 441, 144, 145,
	146, 147, 443, 444, 148, 149, 150, 151, 446, 449, 447, 448, 152, 153, 154,
	155, 450, 451, 156, 157, 158, 159, 453, 468, 454, 461, 455, 458, 456, 457,
	160, 161, 162, 163, 459, 460, 164, 165, 166, 167, 462, 465, 463, 464, 168,
	169, 170, 171, 466, 467, 172, 173, 174, 175, 469, 476, 470, 473, 471, 472,
	176, 177, 178, 179, 474, 475, 180, 181, 182, 183, 477, 480, 478, 479, 184,
	185, 186, 187, 481, 482, 188, 189, 190, 191, 484, 515, 485, 500, 486, 493,
	487, 490, 488, 489, 192, 193, 194, 195, 491, 492, 196, 197, 198, 199, 494,
	497, 495, 496, 200, 201, 202, 203, 498, 499, 204, 205, 206, 207, 501, 508,
	502, 505, 503, 504, 208, 209, 210, 211, 506, 507, 212, 213, 214, 215, 509,
	512, 510, 511, 216, 217, 218, 219, 513, 514, 220, 221, 222, 223, 516, 531,
	517, 524, 518, 521, 519, 520, 224, 225, 226, 227, 522, 523, 228, 229, 230,
	231, 525, 528, 526, 527, 232, 233, 234, 235, 529, 530, 236, 237, 238, 239,
	532, 539, 533, 536, 534, 535, 240, 241, 242, 243, 537, 538, 244, 245, 246,
	247, 540, 543, 541, 542, 248, 249, 250, 251, 544, 545, 252, 253, 254, 255,
	547, 554, 548, 551, 549, 550, 256, 257, 258, 259, 552, 553, 260, 261, 262,
	263, 555, 558, 556, 557, 264, 265, 266, 267, 559, 560, 268, 269, 270, 271,
	562, 565, 563, 564, 272, 273, 274, 275, 566, 567, 276, 277, 278, 279, 569,
	572, 570, 571, 280, 281, 282, 283, 573, 574, 284, 285, 286, 287, 0, 0
};

static unsigned FIXED_DISTANCE_TREE[NUM_DISTANCE_SYMBOLS * 2] = {
	33, 48, 34, 41, 35, 38, 36, 37, 0, 1, 2, 3, 39, 40, 4, 5, 6, 7, 42, 45, 43,
	44, 8, 9, 10, 11, 46, 47, 12, 13, 14, 15, 49, 56, 50, 53, 51, 52, 16, 17,
	18, 19, 54, 55, 20, 21, 22, 23, 57, 60, 58, 59, 24, 25, 26, 27, 61, 62, 28,
	29, 30, 31, 0, 0
};

static unsigned char read_bit(unsigned long *bitpointer, const unsigned char *bitstream) {
	unsigned char result = static_cast<unsigned char>((bitstream[(*bitpointer) >> 3] >> ((*bitpointer) & 0x7)) & 1);
	(*bitpointer)++;
	return result;
}

static unsigned read_bits(unsigned long *bitpointer, const unsigned char *bitstream, unsigned long nbits) {
	unsigned result = 0;
	for (unsigned i = 0; i < nbits; i++) {
		result |= (static_cast<unsigned>(read_bit(bitpointer, bitstream))) << i;
	}
	return result;
}

// Ignore stack usage warnings in GCC for huffman
#pragma GCC diagnostic push
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wstack-usage="
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

// the buffer must be numcodes*2 in size!
void PNG::huffman_tree_init(struct huffman_tree* tree, unsigned* buffer, unsigned numcodes, unsigned maxbitlen) {
	tree->tree2d = buffer;

	tree->numcodes = numcodes;
	tree->maxbitlen = maxbitlen;
}

// given the code lengths (as stored in the PNG file), generate the tree as defined by Deflate.
//  maxbitlen is the maximum bits that a code in the tree can have. return value is error.
void PNG::huffman_tree_create_lengths(struct huffman_tree* tree, const unsigned *bitlen) {
	unsigned tree1d[MAX_SYMBOLS];
	unsigned blcount[MAX_BIT_LENGTH];
	unsigned nextcode[MAX_BIT_LENGTH+1];
	unsigned nodefilled = 0;  // up to which node it is filled
	unsigned treepos = 0;     // position in the tree (1 of the numcodes columns)

	// initialize local vectors
	memset(blcount, 0, sizeof(blcount));
	memset(nextcode, 0, sizeof(nextcode));

	// step 1: count number of instances of each code length
	for (unsigned bits = 0; bits < tree->numcodes; bits++) {
		blcount[bitlen[bits]]++;
	}

	// step 2: generate the nextcode values
	for (unsigned bits = 1; bits <= tree->maxbitlen; bits++) {
		nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;
	}

	// step 3: generate all the codes
	for (unsigned n = 0; n < tree->numcodes; n++) {
		if (bitlen[n] != 0) {
			tree1d[n] = nextcode[bitlen[n]]++;
		}
	}

	/* convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means uninited, a value >= numcodes is an
	 * address to another bit, a value < numcodes is a code. The 2 rows are the 2 possible bit values (0 or 1),
	 * there are as many columns as codes - 1
	 * a good huffmann tree has N * 2 - 1 nodes, of which N - 1 are internal nodes. Here, the internal nodes are
	 * stored (what their 0 and 1 option point to). There is only memory for such good tree currently, if there
	 * are more nodes (due to too long length codes), error 55 will happen
	 */
	for (unsigned n = 0; n < tree->numcodes * 2; n++) {
		tree->tree2d[n] = 32767;  // 32767 here means the tree2d isn't filled there yet
	}

	for (unsigned n = 0; n < tree->numcodes; n++) {  // the codes
		for (unsigned i = 0; i < bitlen[n]; i++) {   // the bits for this code
			unsigned char bit = static_cast<unsigned char>((tree1d[n] >> (bitlen[n] - i - 1)) & 1);
			// check if oversubscribed
			if (treepos > tree->numcodes - 2) {
				SET_ERROR(PNG_EMALFORMED);
				return;
			}

			if (tree->tree2d[2 * treepos + bit] == 32767) {  // not yet filled in
				if (i + 1 == bitlen[n]) {                    // last bit
					tree->tree2d[2 * treepos + bit] = n;     // put the current code in it
					treepos = 0;
				} else {  // put address of the next step in here,
				          // first that address has to be found of course (it's just nodefilled + 1)...
					nodefilled++;
					tree->tree2d[2 * treepos + bit] = nodefilled + tree->numcodes;  // addresses encoded with numcodes added to it
					treepos = nodefilled;
				}
			} else {
				treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes;
			}
		}
	}

	for (unsigned n = 0; n < tree->numcodes * 2; n++) {
		if (tree->tree2d[n] == 32767) {
			tree->tree2d[n] = 0;  // remove possible remaining 32767's
		}
	}
}

unsigned PNG::huffman_decode_symbol(const unsigned char *in, unsigned long *bp,
                                    const huffman_tree* codetree, unsigned long inlength) {
	unsigned treepos = 0;
	while (true) {
		// error: end of input memory reached without endcode
		if (((*bp) & 0x07) == 0 && ((*bp) >> 3) > inlength) {
			SET_ERROR(PNG_EMALFORMED);
			return 0;
		}

		unsigned char bit = read_bit(bp, in);

		unsigned ct = codetree->tree2d[(treepos << 1) | bit];
		if (ct < codetree->numcodes) {
			return ct;
		}

		treepos = ct - codetree->numcodes;
		if (treepos >= codetree->numcodes) {
			SET_ERROR(PNG_EMALFORMED);
			return 0;
		}
	}
}

// get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree
void PNG::get_tree_inflate_dynamic(struct huffman_tree* codetree, struct huffman_tree* codetreeD,
                                   struct huffman_tree* codelengthcodetree, const unsigned char *in,
                                   unsigned long *bp, unsigned long inlength) {
	unsigned codelengthcode[NUM_CODE_LENGTH_CODES];
	unsigned bitlen[NUM_DEFLATE_CODE_SYMBOLS];
	unsigned bitlenD[NUM_DISTANCE_SYMBOLS];

	// make sure that length values that aren't filled in will be 0, or a wrong tree will be generated
	// C-code note: use no "return" between ctor and dtor of an uivector!
	if ((*bp) >> 3 >= inlength - 2) {
		SET_ERROR(PNG_EMALFORMED);
		return;
	}

	// clear bitlen arrays
	memset(bitlen, 0, sizeof(bitlen));
	memset(bitlenD, 0, sizeof(bitlenD));

	// the bit pointer is or will go past the memory
	// number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already
	unsigned hlit = read_bits(bp, in, 5) + 257;
	// number of distance codes. Unlike the spec, the value 1 is added to it here already
	unsigned hdist = read_bits(bp, in, 5) + 1;
	// number of code length codes. Unlike the spec, the value 4 is added to it here already
	unsigned hclen = read_bits(bp, in, 4) + 4;

	for (unsigned i = 0; i < NUM_CODE_LENGTH_CODES; i++) {
		if (i < hclen) {
			codelengthcode[CLCL[i]] = read_bits(bp, in, 3);
		} else {
			codelengthcode[CLCL[i]] = 0;  // if not, it must stay 0
		}
	}

	huffman_tree_create_lengths(codelengthcodetree, codelengthcode);

	// bail now if we encountered an error earlier
	if (error != PNG_EOK) {
		return;
	}

	// now we can use this tree to read the lengths for the tree that this function will return
	unsigned i = 0;
	// i is the current symbol we're reading in the part that contains the code lengths of lit/len codes and dist codes
	while (i < hlit + hdist) {
		unsigned code = huffman_decode_symbol(in, bp, codelengthcodetree, inlength);
		if (error != PNG_EOK) {
			break;
		}

		if (code <= 15) {  // a length code
			if (i < hlit) {
				bitlen[i] = code;
			} else {
				bitlenD[i - hlit] = code;
			}
			i++;
		} else if (code == 16) {     // repeat previous
			unsigned replength = 3;  // read in the 2 bits that indicate repeat length (3-6)
			unsigned value;          // set value to the previous code

			if ((*bp) >> 3 >= inlength) {
				SET_ERROR(PNG_EMALFORMED);
				break;
			}
			// error, bit pointer jumps past memory
			replength += read_bits(bp, in, 2);

			if ((i - 1) < hlit) {
				value = bitlen[i - 1];
			} else {
				value = bitlenD[i - hlit - 1];
			}

			// repeat this value in the next lengths
			for (unsigned n = 0; n < replength; n++) {
				// i is larger than the amount of codes
				if (i >= hlit + hdist) {
					SET_ERROR(PNG_EMALFORMED);
					break;
				}

				if (i < hlit) {
					bitlen[i] = value;
				} else {
					bitlenD[i - hlit] = value;
				}
				i++;
			}
		} else if (code == 17) {     // repeat "0" 3-10 times
			unsigned replength = 3;  // read in the bits that indicate repeat length
			if ((*bp) >> 3 >= inlength) {
				SET_ERROR(PNG_EMALFORMED);
				break;
			}

			// error, bit pointer jumps past memory
			replength += read_bits(bp, in, 3);

			// repeat this value in the next lengths
			for (unsigned n = 0; n < replength; n++) {
				// error: i is larger than the amount of codes
				if (i >= hlit + hdist) {
					SET_ERROR(PNG_EMALFORMED);
					break;
				}

				if (i < hlit) {
					bitlen[i] = 0;
				} else {
					bitlenD[i - hlit] = 0;
				}
				i++;
			}
		} else if (code == 18) {      // repeat "0" 11-138 times
			unsigned replength = 11;  // read in the bits that indicate repeat length
			// error, bit pointer jumps past memory
			if ((*bp) >> 3 >= inlength) {
				SET_ERROR(PNG_EMALFORMED);
				break;
			}

			replength += read_bits(bp, in, 7);

			// repeat this value in the next lengths
			for (unsigned n = 0; n < replength; n++) {
				// i is larger than the amount of codes
				if (i >= hlit + hdist) {
					SET_ERROR(PNG_EMALFORMED);
					break;
				}
				if (i < hlit) {
					bitlen[i] = 0;
				} else {
					bitlenD[i - hlit] = 0;
				}
				i++;
			}
		} else {
			// somehow an unexisting code appeared. This can never happen.
			SET_ERROR(PNG_EMALFORMED);
			break;
		}
	}

	if (error == PNG_EOK && bitlen[256] == 0) {
		SET_ERROR(PNG_EMALFORMED);
	}

	// the length of the end code 256 must be larger than 0
	// now we've finally got hlit and hdist, so generate the code trees, and the function is done
	if (error == PNG_EOK) {
		huffman_tree_create_lengths(codetree, bitlen);
	}
	if (error == PNG_EOK) {
		huffman_tree_create_lengths(codetreeD, bitlenD);
	}
}

// inflate a block with dynamic of fixed Huffman tree
void PNG::inflate_huffman(unsigned char* out, unsigned long outsize, const unsigned char *in,
                          unsigned long *bp, unsigned long *pos, unsigned long inlength, unsigned btype) {
	unsigned codetree_buffer[DEFLATE_CODE_BUFFER_SIZE];
	unsigned codetreeD_buffer[DISTANCE_BUFFER_SIZE];
	unsigned done = 0;

	struct huffman_tree codetree;
	struct huffman_tree codetreeD;

	if (btype == 1) {
		// fixed trees
		huffman_tree_init(&codetree, reinterpret_cast<unsigned*>(FIXED_DEFLATE_CODE_TREE),
		                  NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
		huffman_tree_init(&codetreeD, reinterpret_cast<unsigned*>(FIXED_DISTANCE_TREE),
		                  NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
	} else if (btype == 2) {
		// dynamic trees
		unsigned codelengthcodetree_buffer[CODE_LENGTH_BUFFER_SIZE];
		huffman_tree codelengthcodetree;

		huffman_tree_init(&codetree, codetree_buffer, NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
		huffman_tree_init(&codetreeD, codetreeD_buffer, NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
		huffman_tree_init(&codelengthcodetree, codelengthcodetree_buffer, NUM_CODE_LENGTH_CODES, CODE_LENGTH_BITLEN);
		get_tree_inflate_dynamic(&codetree, &codetreeD, &codelengthcodetree, in, bp, inlength);
	}

	while (done == 0) {
		unsigned code = huffman_decode_symbol(in, bp, &codetree, inlength);
		if (error != PNG_EOK) {
			return;
		}

		if (code == 256) {
			// end code
			done = 1;
		} else if (code <= 255) {
			// literal symbol
			if ((*pos) >= outsize) {
				SET_ERROR(PNG_EMALFORMED);
				return;
			}

			// store output
			out[(*pos)++] = static_cast<unsigned char>(code);
		} else if (code >= FIRST_LENGTH_CODE_INDEX && code <= LAST_LENGTH_CODE_INDEX) {  // length code
			// part 1: get length base
			unsigned long length = LENGTH_BASE[code - FIRST_LENGTH_CODE_INDEX];

			// part 2: get extra bits and add the value of that to length
			unsigned long numextrabits = LENGTH_EXTRA[code - FIRST_LENGTH_CODE_INDEX];

			// error, bit pointer will jump past memory
			if (((*bp) >> 3) >= inlength) {
				SET_ERROR(PNG_EMALFORMED);
				return;
			}
			length += read_bits(bp, in, numextrabits);

			// part 3: get distance code
			unsigned codeD = huffman_decode_symbol(in, bp, &codetreeD, inlength);
			if (error != PNG_EOK) {
				return;
			}

			// invalid distance code (30-31 are never used)
			if (codeD > 29) {
				SET_ERROR(PNG_EMALFORMED);
				return;
			}

			unsigned distance = DISTANCE_BASE[codeD];

			// part 4: get extra bits from distance
			unsigned numextrabitsD = DISTANCE_EXTRA[codeD];

			// error, bit pointer will jump past memory
			if (((*bp) >> 3) >= inlength) {
				SET_ERROR(PNG_EMALFORMED);
				return;
			}

			distance += read_bits(bp, in, numextrabitsD);

			// part 5: fill in all the out[n] values based on the length and dist
			unsigned long start = (*pos);
			unsigned long backward = start - distance;

			if ((*pos) + length >= outsize) {
				SET_ERROR(PNG_EMALFORMED);
				return;
			}

			for (unsigned long forward = 0; forward < length; forward++) {
				out[(*pos)++] = out[backward];
				backward++;

				if (backward >= start) {
					backward = start - distance;
				}
			}
		}
	}
}
#pragma GCC diagnostic pop

void PNG::inflate_uncompressed(unsigned char* out, unsigned long outsize, const unsigned char *in,
                               unsigned long *bp, unsigned long *pos, unsigned long inlength) {
	// go to first boundary of byte
	while (((*bp) & 0x7) != 0) {
		(*bp)++;
	}
	unsigned long p = (*bp) / 8;		// byte position

	// read len (2 bytes) and nlen (2 bytes)
	if (p >= inlength - 4) {
		SET_ERROR(PNG_EMALFORMED);
		return;
	}

	unsigned len = in[p] + 256 * in[p + 1];
	p += 2;
	unsigned nlen = in[p] + 256 * in[p + 1];
	p += 2;

	// check if 16-bit nlen is really the one's complement of len
	if (len + nlen != 65535) {
		SET_ERROR(PNG_EMALFORMED);
		return;
	}

	if ((*pos) + len >= outsize) {
		SET_ERROR(PNG_EMALFORMED);
		return;
	}

	// read the literal data: len bytes are now stored in the out buffer
	if (p + len > inlength) {
		SET_ERROR(PNG_EMALFORMED);
		return;
	}

	for (unsigned n = 0; n < len; n++) {
		out[(*pos)++] = in[p++];
	}

	(*bp) = p * 8;
}

enum PNG::error PNG::uz_inflate_data(unsigned char* out, unsigned long outsize, const unsigned char *in,
                                     unsigned long insize, unsigned long inpos) {
	// bit pointer in the "in" data, current byte is bp >> 3, current bit is bp & 0x7 (from lsb to msb of the byte)
	unsigned long bp = 0;
	// byte position in the out buffer
	unsigned long pos = 0;

	unsigned done = 0;

	while (done == 0) {
		unsigned btype;

		// ensure next bit doesn't point past the end of the buffer
		if ((bp >> 3) >= insize) {
			SET_ERROR(PNG_EMALFORMED);
			return error;
		}

		// read block control bits
		done = read_bit(&bp, &in[inpos]);
		btype = read_bit(&bp, &in[inpos]) | (read_bit(&bp, &in[inpos]) << 1);

		// process control type appropriateyly
		if (btype == 0) {
			inflate_uncompressed(out, outsize, &in[inpos], &bp, &pos, insize);   // no compression
		} else if (btype < 3) {
			inflate_huffman(out, outsize, &in[inpos], &bp, &pos, insize, btype);  // compression, btype 01 or 10
		} else {
			SET_ERROR(PNG_EMALFORMED);
			return error;
		}

		// stop if an error has occured
		if (error != PNG_EOK) {
			return error;
		}
	}

	return error;
}

enum PNG::error PNG::uz_inflate(unsigned char *out, unsigned long outsize,
                                const unsigned char *in, unsigned long insize) {
	// we require two bytes for the zlib data header
	if (insize < 2) {
		SET_ERROR(PNG_EMALFORMED);
		return error;
	}

	// 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way
	if ((in[0] * 256 + in[1]) % 31 != 0) {
		SET_ERROR(PNG_EMALFORMED);
		return error;
	}

	// error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec
	if ((in[0] & 15) != 8 || ((in[0] >> 4) & 15) > 7) {
		SET_ERROR(PNG_EMALFORMED);
		return error;
	}

	// the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset dictionary."
	if (((in[1] >> 5) & 1) != 0) {
		SET_ERROR(PNG_EMALFORMED);
		return error;
	}

	// create output buffer
	uz_inflate_data(out, outsize, in, insize, 2);

	return error;
}

// Paeth predicter, used by PNG filter type 4
static int paeth_predictor(int a, int b, int c) {
	int p = a + b - c;
	int pa = p > a ? p - a : a - p;
	int pb = p > b ? p - b : b - p;
	int pc = p > c ? p - c : c - p;

	if (pa <= pb && pa <= pc) {
		return a;
	} else if (pb <= pc) {
		return b;
	} else {
		return c;
	}
}

void PNG::unfilter_scanline(unsigned char *recon, const unsigned char *scanline, const unsigned char *precon,
                            unsigned long bytewidth, unsigned char filterType, unsigned long length) {
	/*
	   For PNG filter method 0
	   unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte, the filter works byte per byte (bytewidth = 1)
	   precon is the previous unfiltered scanline, recon the result, scanline the current one
	   the incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead
	   recon and scanline MAY be the same memory address! precon must be disjoint.
	 */

	switch (filterType) {
		case 0:
			for (unsigned long i = 0; i < length; i++) {
				recon[i] = scanline[i];
			}
			break;

		case 1:
			for (unsigned long i = 0; i < bytewidth; i++) {
				recon[i] = scanline[i];
			}
			for (unsigned long i = bytewidth; i < length; i++) {
				recon[i] = scanline[i] + recon[i - bytewidth];
			}
			break;

		case 2:
			for (unsigned long i = 0; i < length; i++) {
				recon[i] = scanline[i] + (precon == nullptr ? 0 : precon[i]);
			}
			break;

		case 3:
			for (unsigned long i = 0; i < bytewidth; i++) {
				recon[i] = scanline[i] + (precon == nullptr ? 0 : (precon[i] / 2));
			}
			for (unsigned long i = bytewidth; i < length; i++) {
				recon[i] = scanline[i] + ((recon[i - bytewidth] + (precon == nullptr ? 0 : precon[i])) / 2);
			}
			break;

		case 4:
			for (unsigned long i = 0; i < bytewidth; i++) {
				recon[i] = static_cast<unsigned char>(scanline[i] +
				           (precon == nullptr ? 0 : paeth_predictor(0, precon[i], 0)));
			}
			for (unsigned long i = bytewidth; i < length; i++) {
				recon[i] = static_cast<unsigned char>(scanline[i] + (precon == nullptr ?
				           paeth_predictor(recon[i - bytewidth], 0, 0) :
				           paeth_predictor(recon[i - bytewidth], precon[i], precon[i - bytewidth])));
			}
			break;

		default:
			SET_ERROR(PNG_EMALFORMED);
			break;
	}
}

void PNG::unfilter(unsigned char *out, const unsigned char *in, unsigned w, unsigned h, unsigned bpp) {
	/*
	   For PNG filter method 0
	   this function unfilters a single image (e.g. without interlacing this is called once, with Adam7 it's called 7 times)
	   out must have enough bytes allocated already, in must have the scanlines + 1 filtertype byte per scanline
	   w and h are image dimensions or dimensions of reduced image, bpp is bpp per pixel
	   in and out are allowed to be the same memory address!
	 */

	unsigned y;
	unsigned char *prevline = 0;

	// bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise
	unsigned long bytewidth = (bpp + 7) / 8;
	unsigned long linebytes = (w * bpp + 7) / 8;

	for (y = 0; y < h; y++) {
		unsigned long outindex = linebytes * y;
		unsigned long inindex = (1 + linebytes) * y;  // the extra filterbyte added to each row
		unsigned char filterType = in[inindex];

		unfilter_scanline(&out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes);
		if (error != PNG_EOK) {
			return;
		}

		prevline = &out[outindex];
	}
}

void PNG::remove_padding_bits(unsigned char *out, const unsigned char *in,
                              unsigned long olinebits, unsigned long ilinebits, unsigned h) {
	/*
	   After filtering there are still padding bpp if scanlines have non multiple of 8 bit amounts. They need to be removed (except at last scanline of (Adam7-reduced) image) before working with pure image buffers for the Adam7 code, the color convert code and the output to the user.
	   in and out are allowed to be the same buffer, in may also be higher but still overlapping; in must have >= ilinebits*h bpp, out must have >= olinebits*h bpp, olinebits must be <= ilinebits
	   also used to move bpp after earlier such operations happened, e.g. in a sequence of reduced images from Adam7
	   only useful if (ilinebits - olinebits) is a value in the range 1..7
	 */
	unsigned long diff = ilinebits - olinebits;
	unsigned long obp = 0;
	unsigned long ibp = 0;  // bit pointers
	for (unsigned y = 0; y < h; y++) {
		for (unsigned long x = 0; x < olinebits; x++) {
			unsigned char bit = static_cast<unsigned char>((in[(ibp) >> 3] >> (7 - ((ibp) & 0x7))) & 1);
			ibp++;

			if (bit == 0) {
				out[(obp) >> 3] &= static_cast<unsigned char>(~(1 << (7 - ((obp) & 0x7))));
			} else {
				out[(obp) >> 3] |= (1 << (7 - ((obp) & 0x7)));
			}
			++obp;
		}
		ibp += diff;
	}
}

// out must be buffer big enough to contain full image, and in must contain the
// full decompressed data from the IDAT chunks
void PNG::post_process_scanlines(unsigned char *out, unsigned char *in, const PNG * info_png) {
	unsigned bpp = info_png->get_bpp();
	unsigned w = info_png->width;
	unsigned h = info_png->height;

	if (bpp == 0) {
		SET_ERROR(PNG_EMALFORMED);
		return;
	}

	if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8) {
		unfilter(in, in, w, h, bpp);
		if (error != PNG_EOK) {
			return;
		}
		remove_padding_bits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
	} else {
		unfilter(out, in, w, h, bpp);  // we can immediatly filter into the out buffer, no other steps needed
	}
}

enum PNG::format PNG::determine_format() {
	switch (color_type) {
		case COLOR_LUM:
			switch (color_depth) {
			case 1:
				return LUMINANCE1;
			case 2:
				return LUMINANCE2;
			case 4:
				return LUMINANCE4;
			case 8:
				return LUMINANCE8;
			default:
				return BADFORMAT;
			}
		case COLOR_RGB:
			switch (color_depth) {
			case 8:
				return RGB8;
			case 16:
				return RGB16;
			default:
				return BADFORMAT;
			}
		case COLOR_LUMA:
			switch (color_depth) {
			case 1:
				return LUMINANCE_ALPHA1;
			case 2:
				return LUMINANCE_ALPHA2;
			case 4:
				return LUMINANCE_ALPHA4;
			case 8:
				return LUMINANCE_ALPHA8;
			default:
				return BADFORMAT;
			}
		case COLOR_RGBA:
			switch (color_depth) {
			case 8:
				return RGBA8;
			case 16:
				return RGBA16;
			default:
				return BADFORMAT;
			}
		default:
			return BADFORMAT;
	}
}

void PNG::free_source() {
	if (source.owning != 0) {
		free(reinterpret_cast<void*>(const_cast<unsigned char*>(source.buffer)));
	}

	source.buffer = nullptr;
	source.size = 0;
	source.owning = 0;
}

// read the information from the header and store it in theupng_Info. return value is error
enum PNG::error PNG::header() {
	// if we have an error state, bail now
	if (error != PNG_EOK) {
		return error;
	}

	// if the state is not NEW (meaning we are ready to parse the header), stop now
	if (state != STATE_NEW) {
		return error;
	}

	/* minimum length of a valid PNG file is 29 bytes
	 * FIXME: verify this against the specification, or
	 * better against the actual code below */
	if (source.size < 29) {
		SET_ERROR(PNG_ENOTPNG);
		return error;
	}

	// check that PNG header matches expected value
	if (source.buffer[0] != 137 || source.buffer[1] != 80 || source.buffer[2] != 78 || source.buffer[3] != 71 ||
	    source.buffer[4] != 13 || source.buffer[5] != 10 || source.buffer[6] != 26 || source.buffer[7] != 10) {
		SET_ERROR(PNG_ENOTPNG);
		return error;
	}

	// check that the first chunk is the IHDR chunk
	if (MAKE_DWORD_PTR(source.buffer + 12) != CHUNK_IHDR) {
		SET_ERROR(PNG_EMALFORMED);
		return error;
	}

	// read the values given in the header
	width = MAKE_DWORD_PTR(source.buffer + 16);
	height = MAKE_DWORD_PTR(source.buffer + 20);
	color_depth = source.buffer[24];
	color_type = static_cast<enum PNG::color>(source.buffer[25]);

	// determine our color format
	format = determine_format();
	if (format == BADFORMAT) {
		SET_ERROR(PNG_EUNFORMAT);
		return error;
	}

	// check that the compression method (byte 27) is 0 (only allowed value in spec)
	if (source.buffer[26] != 0) {
		SET_ERROR(PNG_EMALFORMED);
		return error;
	}

	// check that the compression method (byte 27) is 0 (only allowed value in spec)
	if (source.buffer[27] != 0) {
		SET_ERROR(PNG_EMALFORMED);
		return error;
	}

	// check that the compression method (byte 27) is 0 (spec allows 1, but uPNG does not support it)
	if (source.buffer[28] != 0) {
		SET_ERROR(PNG_EUNINTERLACED);
		return error;
	}

	state = STATE_HEADER;
	return error;
}

// read a PNG, the result will be in the same color type as the PNG (hence "generic")
enum PNG::error PNG::decode() {
	unsigned long compressed_size = 0;
	unsigned long compressed_index = 0;

	// if we have an error state, bail now
	if (error != PNG_EOK) {
		return error;
	} else if (state == STATE_DECODED) {
		return PNG_EOK;
	}

	// parse the main header, if necessary
	header();
	if (error != PNG_EOK) {
		return error;
	}

	// if the state is not HEADER (meaning we are ready to decode the image), stop now
	if (state != STATE_HEADER) {
		return error;
	}

	// release old result, if any
	if (buffer != 0) {
		free(buffer);
		buffer = 0;
		size = 0;
	}

	// first byte of the first chunk after the header
	const unsigned char * chunk = source.buffer + 33;

	/* scan through the chunks, finding the size of all IDAT chunks, and also
	 * verify general well-formed-ness */
	while (chunk < source.buffer + source.size) {
		// make sure chunk header is not larger than the total compressed
		if (static_cast<unsigned long>(chunk - source.buffer + 12) > source.size) {
			SET_ERROR(PNG_EMALFORMED);
			return error;
		}

		// get length; sanity check it
		unsigned long length = chunk_length(chunk);
		if (length > __INT_MAX__) {
			SET_ERROR(PNG_EMALFORMED);
			return error;
		}

		// make sure chunk header+paylaod is not larger than the total compressed
		if (static_cast<unsigned long>(chunk - source.buffer + length + 12) > source.size) {
			SET_ERROR(PNG_EMALFORMED);
			return error;
		}

		// parse chunks
		if (chunk_type(chunk) == CHUNK_IDAT) {
			compressed_size += length;
		} else if (chunk_type(chunk) == CHUNK_IEND) {
			break;
		} else if (chunk_critical(chunk)) {
			SET_ERROR(PNG_EUNSUPPORTED);
			return error;
		}

		chunk += chunk_length(chunk) + 12;
	}

	// allocate enough space for the (compressed and filtered) image data
	unsigned char* compressed = compressed_size == 0 ? nullptr : static_cast<unsigned char*>(malloc(compressed_size));
	if (compressed == nullptr) {
		SET_ERROR(PNG_ENOMEM);
		return error;
	}

	/* scan through the chunks again, this time copying the values into
	 * our compressed buffer.  there's no reason to validate anything a second time. */
	chunk = source.buffer + 33;
	while (chunk < source.buffer + source.size) {
		unsigned long length  = chunk_length(chunk);
		const unsigned char *data = chunk + 8;  // the data in the chunk

		// parse chunks
		if (chunk_type(chunk) == CHUNK_IDAT) {
			memcpy(compressed + compressed_index, data, length);
			compressed_index += length;
		} else if (chunk_type(chunk) == CHUNK_IEND) {
			break;
		}

		chunk += chunk_length(chunk) + 12;
	}

	// allocate space to store inflated (but still filtered) data
	unsigned long inflated_size = ((width * (height * get_bpp() + 7)) / 8) + height;
	unsigned char* inflated = static_cast<unsigned char*>(malloc(inflated_size));
	if (inflated == nullptr) {
		free(compressed);
		SET_ERROR(PNG_ENOMEM);
		return error;
	}

	// decompress image data
	enum error err = uz_inflate(inflated, inflated_size, compressed, compressed_size);
	if (err != PNG_EOK) {
		free(compressed);
		free(inflated);
		return error;
	}

	// free the compressed compressed data
	free(compressed);

	// allocate final image buffer
	size = (height * width * get_bpp() + 7) / 8;
	buffer = static_cast<unsigned char*>(malloc(size));
	if (buffer == nullptr) {
		free(inflated);
		size = 0;
		SET_ERROR(PNG_ENOMEM);
		return error;
	}

	// unfilter scanlines
	post_process_scanlines(buffer, inflated, this);
	free(inflated);

	if (error != PNG_EOK) {
		free(buffer);
		buffer = nullptr;
		size = 0;
	} else {
		state = STATE_DECODED;
	}

	// we are done with our input buffer; free it if we own it
	free_source();

	return error;
}

PNG::PNG(const unsigned char* buffer, unsigned long size) : width(0), height(0), color_type(COLOR_RGBA),
         color_depth(8), format(RGBA8), buffer(nullptr), size(0),  state(STATE_NEW), error(PNG_EOK), error_line(0) {
	source.buffer = buffer;
	source.size = size;
	source.owning = 0;

	error = header();
}

PNG::PNG(const char* path) : width(0), height(0), color_type(COLOR_RGBA), color_depth(8), format(RGBA8),
         buffer(nullptr), size(0),  state(STATE_NEW), error(PNG_EOK), error_line(0) {
	source.buffer = nullptr;
	source.size = 0;
	source.owning = 0;
	int fd = VFS::open(path, O_RDONLY);
	struct stat statbuf;
	unsigned char * buf;
	if (fd < 0) {
		SET_ERROR(PNG_ENOTFOUND);
	} else if (VFS::fstat(fd, &statbuf) != 0) {
		SET_ERROR(PNG_EIOERROR);
	} else if ((buf = static_cast<unsigned char*>(malloc(statbuf.st_size + 1))) == nullptr) {
		SET_ERROR(PNG_ENOMEM);
	} else {
		off_t bytes_read = 0;
		while (bytes_read < statbuf.st_size) {
			ssize_t n = VFS::read(fd, buf + bytes_read, statbuf.st_size - bytes_read);
			if (n <= 0) {
				SET_ERROR(PNG_EIOERROR);
				break;
			}
			bytes_read += n;
		}
		if (VFS::close(fd) != 0) {
			SET_ERROR(PNG_EIOERROR);
			free(buf);
		} else {
			source.buffer = buf;
			source.size = bytes_read;
			source.owning = 1;
			error = header();
		}
	}
}

PNG::~PNG() {
	// deallocate image buffer
	if (buffer != nullptr) {
		free(buffer);
	}

	// deallocate source buffer, if necessary
	free_source();
}
