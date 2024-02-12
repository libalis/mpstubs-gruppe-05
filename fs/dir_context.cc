#include "fs/dir_context.h"
#include "fs/errno.h"
#include "fs/util.h"

bool Readdir_Context::dir_emit(const char *name, size_t name_len, ino_t ino, unsigned char type) {
	if (error != 0) {
		return false;
	}

	unsigned short reclen = sizeof(Dirent) + name_len + 1;
	if (buf_used + reclen >= buf_size) {
		if (buf_used == 0) {
			error = -EINVAL;
		}
		return false;
	}

	Dirent dirent = {};
	dirent.d_ino = ino;
	dirent.d_off = 0;
	dirent.d_reclen = reclen;
	dirent.d_type = type;

	Dirent *user_dirent = reinterpret_cast<Dirent *>(buf + buf_used);
	// @Speed we call copy_to_user three times here, the last time for just one byte
	size_t n = copy_to_user(user_dirent, &dirent, sizeof(Dirent));
	if (n != sizeof(dirent)) {
		error = -EFAULT;
		return false;
	}
	n = copy_to_user(user_dirent->d_name, name, name_len);
	if (n != name_len) {
		error = -EFAULT;
		return false;
	}
	char null_byte = 0;
	n = copy_to_user(&user_dirent->d_name[name_len], &null_byte, 1);
	if (n != 1) {
		error = -EFAULT;
		return false;
	}
	buf_used += reclen;
	return true;
}
