#include "fps.h"

#include "syscall/guarded_bell.h"
#include "syscall/guarded_graphics.h"

extern GuardedGraphics graphics;

FPS::FPS() : gout(graphics, Point(graphics.width() - 190, 14), 180, 36, Font::get("Standard", 8, 16)) {}

void FPS::show() {
	gout.draw();
}

void FPS::frame(bool drawn) {
	__atomic_add_fetch(drawn ? &count_drawn : &count_undrawn, 1, __ATOMIC_RELAXED);
}
void FPS::action() {
	while(true) {
		GuardedBell::sleep(1000);
		unsigned drawn = __atomic_exchange_n(&count_drawn, 0, __ATOMIC_RELAXED);
		unsigned total = drawn + __atomic_exchange_n(&count_undrawn, 0, __ATOMIC_RELAXED);
		gout << dec << drawn << " FPS / " << total << " Loops" << endl;
	}
}
