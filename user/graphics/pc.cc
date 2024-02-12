#include "pc.h"

#include "machine/cpuid.h"
#include "syscall/guarded_graphics.h"
#include "syscall/guarded_keyboard.h"

extern GuardedGraphics graphics;
extern GuardedKeyboard keyboard;

PC::PC(const char * image) : image(image), gout(graphics, Point(71, 11), 104, 64, Font::get("Standard", 8, 8)) {
	CPUID::Reg r = CPUID::get(CPUID::MANUFACTURER_ID);
	r.eax = 0;
	gout << r.value << endl << endl << "$> ";
}

void PC::boot() {
	graphics.image(Point(40, 0), image);
	gout.draw();
}

void PC::action() {
	while(true) {
		Key key = keyboard.getKey();
		if(key.valid()) {
			gout << static_cast<char>(key.ascii()) << flush;
		}
	}
}
