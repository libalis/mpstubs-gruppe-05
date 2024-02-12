#pragma once

#include "types.h"

#define MAX_SYMLINK_LEN 4095  // excluding the null byte
#define MAX_DEPTH 6  // symlink lookup is currently recursive!

// Posix definitions taken from linux

#define SEEK_SET	0  /* seek relative to beginning of file */
#define SEEK_CUR	1  /* seek relative to current file position */
#define SEEK_END	2  /* seek relative to end of file */

#define S_IFMT   0170000
#define S_IFSOCK 0140000
#define S_IFLNK  0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

typedef long off_t;
typedef unsigned long ino_t;
typedef unsigned short umode_t;
typedef long time_t;
typedef short uid_t;
typedef char gid_t;
typedef uint32_t nlink_t;
typedef short dev_t;
typedef uint32_t blksize_t;
typedef uint32_t blkcnt_t;

struct stat {
	dev_t     st_dev;     /* ID of device containing file */
	ino_t     st_ino;     /* Inode number */
	umode_t   st_mode;    /* File type and mode */
	nlink_t   st_nlink;   /* Number of hard links */
	uid_t     st_uid;     /* User ID of owner */
	gid_t     st_gid;     /* Group ID of owner */
	dev_t     st_rdev;    /* Device ID (if special file) */
	off_t     st_size;    /* Total size, in bytes */
	blksize_t st_blksize; /* Block size for filesystem I/O */
	blkcnt_t  st_blocks;  /* Number of 512B blocks allocated */
	time_t    st_atime;   /* access timestamp in seconds */
	time_t    st_mtime;   /* modification timestamp in seconds */
	time_t    st_ctime;   /* creation timestamp in seconds */
};

/*
 * File types
 *
 * NOTE! These match bits 12..15 of stat.st_mode
 * (ie "(i_mode >> 12) & 15").
 */
#define DT_UNKNOWN	0
#define DT_FIFO		1
#define DT_CHR		2
#define DT_DIR		4
#define DT_BLK		6
#define DT_REG		8
#define DT_LNK		10
#define DT_SOCK		12
#define DT_WHT		14

struct Dirent {
	ino_t          d_ino;    /* inode number */
	off_t          d_off;    /* offset to next structure */
	unsigned short d_reclen; /* Size of this dirent */
	unsigned char  d_type;   /* File type */
	char           d_name[]; /* Filename (null-terminated) */
};

struct DIR {
	int    fd;               /* Directory file pointer */
	size_t offset;           /* Current offset into the block.  */
	size_t size;             /* Total valid data in the block.  */
	char   buf[4096];        /* Buffer for Dirents */
};

// Open flags
#define O_ACCMODE 00000003
#define O_RDONLY  00000000
#define O_WRONLY  00000001
#define O_RDWR    00000002
#define O_CREAT   00000100
#define O_EXCL    00000200
