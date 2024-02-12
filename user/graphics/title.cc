#include "title.h"

#include "syscall/guarded_vfs.h"
#include "syscall/guarded_graphics.h"
extern GuardedGraphics graphics;

#include "debug/output.h"

Title::Title(const char * file, const Color &color) : color(color), len(0) {
	buffer[0] = '\0';
	int error;
	int fd = GuardedVFS::open(file, O_RDONLY);
	if (fd < 0) {
		DBG_VERBOSE << "open error: " << -fd << endl;
	} else {
		while(len < static_cast<int>(sizeof(buffer)) - 1) {
			ssize_t n = GuardedVFS::read(fd, buffer + len, sizeof(buffer) - len - 1);
			if (n < 0) {
				DBG_VERBOSE << "read error: " << -n << endl;
				break;
			} else if (n == 0) {
				break;
			}
			len += n;
		}
		if ((error = GuardedVFS::close(fd)) != 0) {
			DBG_VERBOSE << "close error: " << -error << endl;
		} else {
			buffer[sizeof(buffer) - 1] = buffer[len] = '\0';
		}
	}
	font = Font::get("Terminus");
}

void Title::print(const Point &p) {
	graphics.text(p, buffer, len - 1, color, font);
	color.red.value += 1;
	color.green.value += 2;
	color.blue.value -= 1;
}
