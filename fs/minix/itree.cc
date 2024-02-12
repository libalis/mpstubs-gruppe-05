// SPDX-License-Identifier: GPL-2.0

#include "fs/minix/minix.h"
#include "fs/errno.h"
#include "utils/string.h"

// @Synchronization I think the synchronization here was only to guard against
// parallel truncates

// @Cleanup
// @Cleanup
// @Cleanup

static inline unsigned long block_to_cpu(block_t n) {
	return n;
}

static inline block_t cpu_to_block(unsigned long n) {
	return n;
}

static inline block_t *i_data(Inode *inode) {
	return reinterpret_cast<block_t *>(minix_i(inode)->data);
}

int Minix::block_to_path(unsigned long block, int offsets[DEPTH]) {
	const unsigned long DIRCOUNT = 7;
	const unsigned long INDIRCOUNT = (1 << (bdev->blocksize_bits - 2));
	int n = 0;

	if (block < DIRCOUNT) {
		offsets[n++] = block;
	} else if ((block -= DIRCOUNT) < INDIRCOUNT) {
		offsets[n++] = DIRCOUNT;
		offsets[n++] = block;
	} else if ((block -= INDIRCOUNT) < INDIRCOUNT * INDIRCOUNT) {
		offsets[n++] = DIRCOUNT + 1;
		offsets[n++] = block / INDIRCOUNT;
		offsets[n++] = block % INDIRCOUNT;
	} else {
		block -= INDIRCOUNT * INDIRCOUNT;
		offsets[n++] = DIRCOUNT + 2;
		offsets[n++] = (block / INDIRCOUNT) / INDIRCOUNT;
		offsets[n++] = (block / INDIRCOUNT) % INDIRCOUNT;
		offsets[n++] = block % INDIRCOUNT;
	}
	return n;
}

// static DEFINE_RWLOCK(pointers_lock);

static inline void add_chain(Indirect *p, Block *block, block_t *v) {
	p->key = *(p->p = v);
	if (block != nullptr) {
		p->block = *block;
	}
}

static inline int verify_chain(Indirect *from, Indirect *to) {
	while (from <= to && from->key == *from->p) {
		from++;
	}
	return static_cast<int>(from > to);
}

static inline block_t *block_end(Block *block) {
	return reinterpret_cast<block_t *>(reinterpret_cast<char*>(block->data) + block->get_size());
}

inline Indirect *Minix::get_branch(Inode *inode, int depth, int *offsets, Indirect chain[DEPTH], int *err) {
	Indirect *p = chain;

	*err = 0;
	/* i_data is not going away, no lock needed */
	add_chain(chain, nullptr, i_data(inode) + *offsets);
	if (p->key == 0) {
		return p;
	}
	while (--depth > 0) {
		Block block = bdev->fix(block_to_cpu(p->key));
		if (block.data == nullptr) {
			*err = block.flags;
			return p;
		}
		// read_lock(&pointers_lock);
		if (verify_chain(chain, p) == 0) {
			block.unfix();
			// read_unlock(&pointers_lock);
			*err = -EAGAIN;
			return p;
		}
		add_chain(++p, &block, reinterpret_cast<block_t *>(block.data) + *++offsets);
		// read_unlock(&pointers_lock);
		if (p->key == 0) {
			return p;
		}
	}
	return nullptr;
}

int Minix::alloc_branch(int num, int *offsets, Indirect *branch) {
	int n = 0;
	int i;
	int parent = new_block();

	branch[0].key = cpu_to_block(parent);
	if (parent != 0) {
		for (n = 1; n < num; n++) {
			/* Allocate the next block */
			int nr = new_block();
			if (nr == 0) {
				break;
			}
			branch[n].key = cpu_to_block(nr);
			Block block = bdev->fix(parent);
			memset(block.data, 0, block.get_size());
			block.mark_dirty();
			branch[n].block = block;
			branch[n].p = reinterpret_cast<block_t *>(block.data) + offsets[n];
			*branch[n].p = branch[n].key;
			parent = nr;
		}
	}
	if (n == num) {
		return 0;
	}

	/* Allocation failed, free what we already allocated */
	for (i = 1; i < n; i++) {
		branch[i].block.forget();
	}
	for (i = 0; i < n; i++) {
		free_block(block_to_cpu(branch[i].key));
	}
	return -ENOSPC;
}

static inline int splice_branch(Inode *inode, Indirect chain[DEPTH], Indirect *where, int num) {
	int i;

	// write_lock(&pointers_lock);

	/* Verify that place we are splicing to is still there and vacant */
	if (verify_chain(chain, where - 1) == 0 || *where->p != 0) {
		goto changed;
	}

	*where->p = where->key;

	// write_unlock(&pointers_lock);

	/* We are done with atomic stuff, now do the rest of housekeeping */

	// @Incomplete timestamp
	// inode->i_ctime = current_time(inode);

	/* had we spliced it onto indirect block? */
	if (where->block.data != nullptr) {
		where->block.mark_dirty();
	}

	inode->mark_dirty();
	return 0;

changed:
	// write_unlock(&pointers_lock);
	for (i = 1; i < num; i++) {
		where[i].block.forget();
	}
	// @Incomplete we don't have free_block currently
	/*
	for (i = 0; i < num; i++)
		minix_free_block(inode, block_to_cpu(where[i].key));
	*/
	return -EAGAIN;
}

uint64_t Minix::get_block(Inode *inode, uint64_t logical_block, bool create, int *error) {
	// @Cleanup
	// @Cleanup
	// @Cleanup
	// @Cleanup
	// @Cleanup
	uint64_t result = 0;
	int offsets[DEPTH];
	Indirect chain[DEPTH];
	Indirect *partial;
	int left;
	int depth = block_to_path(logical_block, offsets);

	*error = -EIO;
	if (depth == 0) {
		goto out;
	}

reread:
	partial = get_branch(inode, depth, offsets, chain, error);

	/* Simplest case - block found, no allocation needed */
	if (partial == nullptr) {
got_it:
		result = block_to_cpu(chain[depth - 1].key);
		/* Clean up and exit */
		partial = chain + depth - 1; /* the whole chain */
		goto cleanup;
	}

	/* Next simple case - plain lookup or failed read of indirect block */
	if (!create || *error == -EIO) {
cleanup:
		while (partial > chain) {
			partial->block.unfix();
			partial--;
		}
out:
		return result;
	}

	/*
	 * Indirect block might be removed by truncate while we were
	 * reading it. Handling of that case (forget what we've got and
	 * reread) is taken out of the main path.
	 */
	if (*error == -EAGAIN) {
		goto changed;
	}

	left = (chain + depth) - partial;
	*error = alloc_branch(left, offsets + (partial - chain), partial);
	if (*error != 0) {
		goto cleanup;
	}

	if (splice_branch(inode, chain, partial, left) < 0) {
		goto changed;
	}

	// set_buffer_new(bh); What did this do???
	goto got_it;

changed:
	while (partial > chain) {
		partial->block.unfix();
		partial--;
	}
	goto reread;
}

static inline int all_zeroes(block_t *p, const block_t *q) {
	while (p < q) {
		if (*p++ != 0) {
			return 0;
		}
	}
	return 1;
}

Indirect *Minix::find_shared(Inode *inode, int depth, int offsets[DEPTH],
                             Indirect chain[DEPTH], block_t *top) {
	Indirect *partial;
	Indirect *p;
	int k;
	int err;

	*top = 0;
	for (k = depth; k > 1 && offsets[k-1] == 0; k--) {}
	partial = get_branch(inode, k, offsets, chain, &err);

	// write_lock(&pointers_lock);
	if (partial == nullptr) {
		partial = chain + k - 1;
	}
	if (partial->key == 0 && *partial->p != 0) {
		// write_unlock(&pointers_lock);
		goto no_top;
	}
	for (p = partial; p > chain && all_zeroes(reinterpret_cast<block_t*>(p->block.data), p->p) != 0; p--) {}

	if (p == chain + k - 1 && p > chain) {
		p->p--;
	} else {
		*top = *p->p;
		*p->p = 0;
	}
	// write_unlock(&pointers_lock);

	while(partial > p) {
		partial->block.unfix();
		partial--;
	}
no_top:
	return partial;
}

void Minix::free_data(block_t *p, const block_t *q) {
	for (unsigned long nr ; p < q ; p++) {
		nr = block_to_cpu(*p);
		if (nr != 0) {
			*p = 0;
			free_block(nr);
		}
	}
}

void Minix::free_branches(Inode *inode, block_t *p, block_t *q, int depth) {
	if (depth-- != 0) {
		for ( ; p < q ; p++) {
			unsigned long nr = block_to_cpu(*p);
			if (nr == 0) {
				continue;
			}
			*p = 0;
			Block block = bdev->fix(nr);
			if (block.data == nullptr) {
				continue;
			}
			free_branches(inode, reinterpret_cast<block_t*>(block.data), block_end(&block), depth);
			block.forget();
			free_block(nr);
			inode->mark_dirty();
		}
	} else {
		free_data(p, q);
	}
}

void Minix::truncate(Inode *inode, off_t length) {
	block_t *idata = i_data(inode);
	int offsets[DEPTH];
	Indirect chain[DEPTH];
	Indirect *partial;
	block_t nr = 0;
	int n;
	int first_whole;
	long iblock;

	inode->size = length;
	inode->mark_dirty();

	iblock = (inode->size + bdev->blocksize -1) >> bdev->blocksize_bits;

	n = block_to_path(iblock, offsets);
	if (n == 0) {
		return;
	}

	if (n == 1) {
		free_data(idata+offsets[0], idata + DIRECT);
		first_whole = 0;
		goto do_indirects;
	}

	first_whole = offsets[0] + 1 - DIRECT;
	partial = find_shared(inode, n, offsets, chain, &nr);
	if (nr != 0) {
		if (partial == chain) {
			inode->mark_dirty();
		} else {
			partial->block.mark_dirty();
		}
		free_branches(inode, &nr, &nr+1, (chain+n-1) - partial);
	}
	/* Clear the ends of indirect blocks on the shared branch */
	while (partial > chain) {
		free_branches(inode, partial->p + 1, block_end(&partial->block),
				(chain+n-1) - partial);
		partial->block.mark_dirty();
		partial->block.unfix();
		partial--;
	}
do_indirects:
	/* Kill the remaining (whole) subtrees */
	while (first_whole < DEPTH-1) {
		nr = idata[DIRECT+first_whole];
		if (nr != 0) {
			idata[DIRECT+first_whole] = 0;
			inode->mark_dirty();
			free_branches(inode, &nr, &nr+1, first_whole+1);
		}
		first_whole++;
	}
	// inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->mark_dirty();
}
