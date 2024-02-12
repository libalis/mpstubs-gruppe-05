#include "user/graphics/example.h"

#include "syscall/guarded_graphics.h"
#include "syscall/guarded_scheduler.h"
#include "syscall/guarded_bell.h"

extern GuardedGraphics graphics;

void GraphicsExample::action() {
	fire.ignite();
	GuardedScheduler::ready(&fire);
	GuardedScheduler::ready(&fps);
	cat.attract();
	pong.start();
	GuardedScheduler::ready(&pc);
	while (true) {
		graphics.clear();
		pc.boot();
		fire.burn();
		demon.summon();
		cat.walk();
		title.print();
		pong.play();
		fps.show();
		fps.frame(graphics.switchBuffers());
		// Optional: Limit the drawing speed
		// GuardedBell::sleep(10);
	}
}
