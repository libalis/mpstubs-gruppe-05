#include "pong.h"

#include "syscall/guarded_graphics.h"
extern GuardedGraphics graphics;

#include "assets/i4.h"

void Pong::start() {
	width = graphics.width() - 1;
	height = graphics.height() * 3 / 4;
	pos.x = width / 2;
	pos.y = height / 2;
}

void Pong::play() {
	pos += dir;
	// Check sides
	if (pos.x <= offset + paddle.x || pos.x >= width - size - offset - paddle.x) {
		if (pos.x < offset + paddle.x) {
			pos.x = offset + paddle.x;
		} else if (pos.x > width - size - offset - paddle.x) {
			pos.x = width - size - offset - paddle.x;
		}
		dir.x = ((rand.number() % 2) + 1) * (dir.x < 0 ? 1 : -1);
		dir.y = (rand.number() % 3) - 2;
	}
	// Check top & bottom
	if (pos.y <= 32 || pos.y >= height - size) {
		if (pos.y < 32) {
			pos.y = 32;
		} else if (pos.y > height - size) {
			pos.y = height - size;
		}
		dir.y *= -1;
	}
	// draw image
	graphics.image(pos, reinterpret_cast<const struct GIMP &>(logo));
	// alternative: graphics.image(pos, reinterpret_cast<const SpritePixelRGBA8*>(logo.pixel_data), size, size);

	Point paddle_left(offset, pos.y);
	graphics.rectangle(paddle_left, paddle_left + paddle, Color(0xff, 0xff, 0xff));

	Point paddle_right(width - offset - paddle.x, pos.y);
	graphics.rectangle(paddle_right, paddle_right + paddle, Color(0xff, 0xff, 0xff));
}
