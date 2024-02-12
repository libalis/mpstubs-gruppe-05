#include "demon.h"

#include "syscall/guarded_graphics.h"
extern GuardedGraphics graphics;

Demon::Demon(const char * image) : image(image) { }

void Demon::summon() {
	Point pos((graphics.width() - image.get_width())/2, graphics.height() - image.get_height() - 50);
	graphics.image(pos, image);
}
