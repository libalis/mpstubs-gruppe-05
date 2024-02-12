#pragma once

#include "fs/blockdevice.h"
#include "fs/vfs.h"  // for S_ISDIR
#include "fs/filesystem.h"
#include "fs/inode.h"
#include "fs/block.h"

#define MINIX_ROOT_INO 1
#define MINIX3_SUPER_MAGIC 0x4d5a
// from fs.h
#define BLOCK_SIZE_BITS 10
#define BLOCK_SIZE (1 << BLOCK_SIZE_BITS)

enum {DIRECT = 7, DEPTH = 4};	/* Have triple indirect */
typedef uint32_t block_t;	/* 32 bit, host order */

struct Indirect {
	block_t	*p;
	block_t	key;
	Block block;
};

#define minix_i(inode) (static_cast<MinixInode *>(inode))

struct Minix_Disk_Inode {
	uint16_t mode;
	uint16_t nlinks;
	uint16_t uid;
	uint16_t gid;
	uint32_t size;
	uint32_t atime;
	uint32_t mtime;
	uint32_t ctime;
	uint32_t zone[10];
};

struct Minix_Super_Block {
	uint32_t ninodes;
	uint16_t pad0;
	uint16_t imap_blocks;
	uint16_t zmap_blocks;
	uint16_t firstdatazone;
	uint16_t log_zone_size;
	uint16_t pad1;
	uint32_t max_size;
	uint32_t nzones;
	uint16_t magic;
	uint16_t pad2;
	uint16_t blocksize;
	uint8_t  disk_version;
};

struct Minix_Dirent {
	uint32_t inode;
	char name[60];
};

class MinixInode;

class Minix: public Filesystem {
	friend MinixInode;

 public:
	Minix() {}
	~Minix() {}

	int mount(const void *data);
	int sync();
	void umount();
	int open(File *file);
	int close(File *file);
	uint64_t get_block(Inode *inode, uint64_t logical_block, bool create, int *error);
	Inode *create(Inode *dir, const char *filename, size_t name_len, umode_t mode, int *error);
	int link(Inode *dir, const char *filename, size_t name_len, Inode *inode);
	int symlink(Inode *dir, const char *filename, size_t name_len, const char *symname);
	int unlink(Inode *dir, const char *filename, size_t name_len, Inode *inode);
	void truncate(Inode *inode, off_t length);
	Inode *lookup(Inode *dir, const char *filename, size_t name_len, int *error);
	int iterate_dir(Inode *dir, Dir_Context *ctx);
	int mkdir(Inode *parent_dir, const char *filename, size_t name_len, umode_t mode);
	int rmdir(Inode *parent_dir, const char *filename, size_t name_len, Inode *dir);
	int rename(Inode *old_dir, Inode *old_inode, const char *old_name, size_t old_name_len,
	           Inode *new_dir, Inode *new_inode, const char *new_name, size_t new_name_len);
	int write_inode(Inode *inode);
	Inode *allocate_inode();

 private:
	Minix_Super_Block *super;
	Block super_block;
	Block *imap;
	Block *zmap;

	Inode *iget(unsigned long ino, int *error);
	int new_block();
	Minix_Disk_Inode *raw_inode(ino_t ino, Block *block, int *error);
	Inode *new_inode(umode_t mode, int *error);
	int block_to_path(unsigned long block, int offsets[DEPTH]);
	Indirect *get_branch(Inode *inode, int depth, int *offsets, Indirect chain[DEPTH], int *err);
	int alloc_branch(int num, int *offsets, Indirect *branch);
	Minix_Dirent *find_dirent(Inode *dir, const char *filename, size_t name_len, Block *p_block,
	                          int *error);
	int add_link(Inode *dir, Inode *inode, const char *name, size_t name_len);
	int delete_entry(Inode *dir, const char *filename, size_t name_len);
	int make_empty_dir(Inode *inode, Inode *parent_dir);
	void free_branches(Inode *inode, block_t *p, block_t *q, int depth);
	Indirect *find_shared(Inode *inode, int depth, int offsets[DEPTH],
			Indirect chain[DEPTH], block_t *top);
	void free_data(block_t *p, const block_t *q);
	void free_block(unsigned long block);
	void clear_disk_inode(Inode *inode);
	void free_inode(Inode *inode);
	int check_dir_is_empty(Inode *dir);
	inline unsigned int dir_block_last_byte(Inode *dir, unsigned long lblock);
};

class MinixInode : public Inode {
 public:
	uint32_t data[16];

	explicit MinixInode(Filesystem *fs) : Inode(fs) {
		for (int i = 0; i < 16; i++) {
			data[i] = 0;
		}
	}

	~MinixInode() {
		if (nlinks == 0) {
			Minix *fs = static_cast<Minix *>(filesystem);
			fs->truncate(this, 0);
			fs->free_inode(this);
			return;
		}

		if (!is_new() && is_dirty()) {
			write_to_disk();
		}
	}
};
