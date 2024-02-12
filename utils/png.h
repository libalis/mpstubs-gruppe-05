/*! \file
 * \brief \ref PNG library
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

#pragma once

/**! \brief Portable Network Graphics file format
 */
class PNG {
 public:
	enum error {
		PNG_EOK           = 0,  ///< success (no error)
		PNG_ENOMEM        = 1,  ///< memory allocation failed
		PNG_ENOTFOUND     = 2,  ///< resource not found (file missing)
		PNG_ENOTPNG       = 3,  ///< image data does not have a PNG header
		PNG_EMALFORMED    = 4,  ///< image data is not a valid PNG image
		PNG_EUNSUPPORTED  = 5,  ///< critical PNG chunk type is not supported
		PNG_EUNINTERLACED = 6,  ///< image interlacing is not supported
		PNG_EUNFORMAT     = 7,  ///< image color format is not supported
		PNG_EPARAM        = 8,  ///< invalid parameter to method call
		PNG_EIOERROR      = 9   ///< Filesystem issues
	};

	enum format {
		BADFORMAT,         ///< Invalid format
		RGB8,              ///< 24bit RGB (8bit per color)
		RGB16,             ///< 48bit RGB (16bit per color)
		RGBA8,             ///< 32bit RGB (including alpha channel)
		RGBA16,            ///< 64bit RGB (including alpha channel)
		LUMINANCE1,        ///< Monochrome (black-white)
		LUMINANCE2,        ///< four shades of gray (2bit gray scale)
		LUMINANCE4,        ///< 16 shades of gray (4bit gray scale)
		LUMINANCE8,        ///< 8bit gray scale
		LUMINANCE_ALPHA1,  ///< Monochrome (black-white) with alpha channel
		LUMINANCE_ALPHA2,  ///< 2bit gray scale with alpha channel
		LUMINANCE_ALPHA4,  ///< 4bit gray scale with alpha channel
		LUMINANCE_ALPHA8   ///< 8bit gray scale with alpha channel
	};

 private:
	enum color {
		COLOR_LUM = 0,
		COLOR_RGB = 2,
		COLOR_LUMA = 4,
		COLOR_RGBA = 6
	};

	unsigned width = 0;
	unsigned height = 0;

	enum color color_type;
	unsigned color_depth;
	enum format format = BADFORMAT;

	unsigned char* buffer = nullptr;
	unsigned long size = 0;

	enum {
		STATE_ERROR = -1,
		STATE_DECODED = 0,
		STATE_HEADER = 1,
		STATE_NEW = 2
	} state;

	struct {
		const unsigned char* buffer;
		unsigned long size;
		char owning;
	} source;

	struct huffman_tree {
		unsigned* tree2d;
		unsigned maxbitlen;  ///< maximum number of bits a single code can get
		unsigned numcodes;   ///< number of symbols in the alphabet = number of codes
	};

	void free_source();
	static void huffman_tree_init(struct huffman_tree* tree, unsigned* buffer, unsigned numcodes, unsigned maxbitlen);
	void huffman_tree_create_lengths(struct huffman_tree* tree, const unsigned* bitlen);
	unsigned huffman_decode_symbol(const unsigned char* in, unsigned long* bp,
	                               const struct huffman_tree* codetree, unsigned long inlength);
	void get_tree_inflate_dynamic(struct huffman_tree* codetree, struct huffman_tree* codetreeD,
	                              struct huffman_tree* codelengthcodetree, const unsigned char* in,
	                              unsigned long* bp, unsigned long inlength);
	void inflate_huffman(unsigned char* out, unsigned long outsize, const unsigned char* in, unsigned long* bp,
	                     unsigned long* pos, unsigned long inlength, unsigned btype);
	void inflate_uncompressed(unsigned char* out, unsigned long outsize, const unsigned char* in, unsigned long* bp,
	                          unsigned long* pos, unsigned long inlength);
	enum error uz_inflate_data(unsigned char* out, unsigned long outsize, const unsigned char* in,
                               unsigned long insize, unsigned long inpos);
	enum error uz_inflate(unsigned char* out, unsigned long outsize, const unsigned char* in, unsigned long insize);
	void unfilter_scanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon,
	                       unsigned long bytewidth, unsigned char filterType, unsigned long length);
	void unfilter(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp);
	static void remove_padding_bits(unsigned char* out, const unsigned char* in,
	                                unsigned long olinebits, unsigned long ilinebits, unsigned h);
	void post_process_scanlines(unsigned char* out, unsigned char* in, const PNG* info_png);
	enum format determine_format();

 protected:
	enum error error;
	unsigned error_line;

	/*! \brief Extract header (image attributes)
	 */
	enum error header();

	/*! \brief Decode whole image
	 */
	enum error decode();

 public:
	/*! \brief Load PNG image from memory
	 *  \param buffer pointer to memory buffer
	 *  \param size size of memory buffer
	 */
	PNG(const unsigned char* buffer, unsigned long size);

	/*! \brief Load PNG image from file system
	 *  \param path path to file
	 */
	explicit PNG(const char* path);

	/*! \brief Free memory reserved for PNG image
	 */
	~PNG();

	/*! \brief Width of image (pixels)
	 */
	unsigned get_width() const {
		return width;
	}

	/*! \brief Height of image (pixels)
	 */
	unsigned get_height() const {
		return height;
	}

	/*! \brief Bits per pixel
	 */
	unsigned get_bpp() const {
		return get_bitdepth() * get_components();
	}

	/*! \brief Depth of color (bits pro color channel)
	 */
	unsigned get_bitdepth() const {
		return color_depth;
	}

	/*! \brief Number of components per pixel
	 */
	unsigned get_components() const {
		switch (color_type) {
		case COLOR_LUM:
			return 1;
		case COLOR_RGB:
			return 3;
		case COLOR_LUMA:
			return 2;
		case COLOR_RGBA:
			return 4;
		default:
			return 0;
		}
	}

	/*! \brief Number of bytes per pixel
	 */
	unsigned get_pixelsize() const {
		unsigned bits = get_bitdepth() * get_components();
		bits += bits % 8;
		return bits;
	}

	/*! \brief Retrieve the format
	 */
	enum format get_format() const {
		return format;
	}

	/*! \brief Image buffer address
	 * \return Pointer to image buffer
	 */
	const unsigned char* get_buffer() {
		decode();
		return buffer;
	}

	/*! \brief Size of image buffer
	 * \return Total size of image buffer in bytes
	 */
	unsigned get_size() {
		decode();
		return size;
	}
};
