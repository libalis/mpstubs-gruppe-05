#include "user/graphics/fire.h"

#include "utils/random.h"
#include "syscall/guarded_graphics.h"
#include "syscall/guarded_scheduler.h"
extern GuardedGraphics graphics;

static Random rand(42);
static Color palette[256];

static unsigned fire[Fire::max_height + 1][Fire::max_width];
static Color image[Fire::max_height * Fire::max_width];

// Helper to convert HSL to RGB
static Color hslColor(int h, int s, int l) {
	int v = (l < 128) ? (l * (256 + s)) >> 8 : (((l + s) << 8) - l * s) >> 8;
	if (v > 0) {
		int m = l + l - v;
		h *= 6;
		int sextant = h >> 8;
		int fract = (h - (sextant << 8));
		int vsf = v * fract * (v - m) / v >> 8;
		int mid1 = m + vsf;
		int mid2 = v - vsf;
		switch (sextant) {
			case 0:
				return Color(v, mid1, m);
			case 1:
				return Color(mid2, v, m);
			case 2:
				return Color(m, v, mid1);
			case 3:
				return Color(m, mid2, v);
			case 4:
				return Color(mid1, m, v);
			default:
				return Color(v, m, mid2);
		}
	}
	return Color(0, 0, 0);
}

Fire::Fire() : height(max_height), width(max_width), offset(0, max_height) {
	// generate the fire palette
	for(unsigned x = 0; x < 256; x++) {
		palette[x] = hslColor(x / 3, 255, x >= 127 ? 255 : x * 2);
	}
	for (unsigned y = 0 ; y <= height; y++) {
		for (unsigned x = 0 ; x < width; x++) {
			fire[y][x] = 0;
		}
	}
}

static int abs(int a) {
	return (a >= 0 ? a : -a);
}

void Fire::ignite() {
	height = graphics.height();
	if (height > max_height) {
		offset.y = height - max_height;
		height = max_height;
	} else {
		offset.y = 0;
	}
	width = graphics.width();
	if (width > max_width) {
		width = max_width;
	}
}

void Fire::action() {
	while (true) {
		for(unsigned x = 0; x < width; x++) {
			if ((rand.number() % 100) == 0) {
				fire[0][x] = abs(rand.number()) % 256;
			}
		}

		for(unsigned y = height; y > 0; y--) {
			for(unsigned x = 0; x < width; x++) {
				fire[y][x] = ((fire[y - 1][(x - 1 + width) % width]
				           + fire[y > 2 ? (y - 2) : 0][(x) % width]
				           + fire[y - 1][(x + 1) % width]
				           + fire[y > 3 ? (y - 3) : 0][(x) % width])
				           * 64) / 257;
			}
		}

		for(unsigned y = 0; y < height; y++) {
			for(unsigned x = 0; x < width; x++) {
				image[(height - y - 1) * width + x] = palette[fire[y][x]];
			}
		}
	}
}

void Fire::burn() {
	graphics.image(offset, image, width, height);
}
