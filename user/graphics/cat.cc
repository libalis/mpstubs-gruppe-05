#include "cat.h"

#include "syscall/guarded_graphics.h"
extern GuardedGraphics graphics;

Cat::Cat(const char * image) : x(1024), y(550), animationCurrent(0), position(-800), image(image) {
}

void Cat::attract() {
	x = graphics.width();
	y = graphics.height() - height - 28;
}

void Cat::walk() {
	graphics.image(Point(position, y), image, 0, height, 0, height * animationCurrent);
	if (((position += 2) % 20) == 0) {
		animationCurrent = (animationCurrent + 1) % animationCount;
	}
	if (position > x) {
		position = -width;
	}
}
