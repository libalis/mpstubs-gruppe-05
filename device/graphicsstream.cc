#include "device/graphicsstream.h"

#include "graphics/fonts/font.h"

#include "utils/alloc.h"
#include "utils/math.h"

const Color GraphicsStream::BLACK(0x00, 0x00, 0x00);
const Color GraphicsStream::BLUE(0x00, 0x00, 0xAA);
const Color GraphicsStream::GREEN(0x00, 0xAA, 0x00);
const Color GraphicsStream::CYAN(0x00, 0xAA, 0xAA);
const Color GraphicsStream::RED(0xAA, 0x00, 0x00);
const Color GraphicsStream::MAGENTA(0xAA, 0x00, 0xAA);
const Color GraphicsStream::BROWN(0xAA, 0x55, 0x00);
const Color GraphicsStream::LIGHT_GREY(0xAA, 0xAA, 0xAA);

const Color GraphicsStream::DARK_GREY(0x55, 0x55, 0x55);
const Color GraphicsStream::LIGHT_BLUE(0x55, 0x55, 0xFF);
const Color GraphicsStream::LIGHT_GREEN(0x55, 0xFF, 0x55);
const Color GraphicsStream::LIGHT_CYAN(0x55, 0xFF, 0xFF);
const Color GraphicsStream::LIGHT_RED(0xFF, 0x55, 0x55);
const Color GraphicsStream::LIGHT_MAGENTA(0xFF, 0x55, 0xFF);
const Color GraphicsStream::YELLOW(0xFF, 0xFF, 0x55);
const Color GraphicsStream::WHITE(0xFF, 0xFF, 0xFF);

GraphicsStream::GraphicsStream(Graphics &graphics, const Point &start, unsigned width, unsigned height,
                                Font * const font)
                              : offset(0), graphics(graphics), x(0), y(0), FONT(font == nullptr ? Font::get() : font),
                                START(start), ROWS(height / this->FONT->height), COLUMNS(width / this->FONT->width) {
                                cell = reinterpret_cast<Cell *>(calloc(ROWS * COLUMNS, sizeof(Cell)));
}

void GraphicsStream::setPos(int x, int y) {
	if (x < 0) {
		x += COLUMNS;
	}
	if (y < 0) {
		y += ROWS;
	}

	if (x >= 0 && static_cast<unsigned>(x) < COLUMNS &&
	    y >= 0 && static_cast<unsigned>(y) <= ROWS) {
		this->x = x;
		this->y = y;
	}
}

void GraphicsStream::getPos(int& x, int& y) const {
	x = this->x;
	y = this->y;
}

void GraphicsStream::show(int x, int y, char character, const Color &color) {
	if (x < 0) {
		x += COLUMNS;
	}
	if (y < 0) {
		y += ROWS;
	}
	// only print if position within the screen range
	if (x >= 0 && static_cast<unsigned>(x) < COLUMNS &&
	    y >= 0 && static_cast<unsigned>(y) < ROWS) {
		cell[((offset + y) % ROWS) * COLUMNS + x] = (Cell){ character, color};
	}
}

void GraphicsStream::print(char* str, int length, const Color &color) {
	while (length > 0) {
		switch (*str) {
			case '\n':
				for (unsigned i = x; i < COLUMNS; ++i) {
					show(i, y, ' ', color);
				}
				x = 0;
				y++;
				break;

			default:
				show(x, y, *str, color);
				if (++x >= COLUMNS) {
					x = 0;
					y++;
				}
				break;
		}
		str++;
		if (y >= ROWS) {
			offset = (offset + 1) % ROWS;
			y--;
			for (unsigned i = 0; i < COLUMNS; ++i) {
				show(i, y, ' ', color);
			}
		}
		length--;
	 }
}

void GraphicsStream::reset(char character, const Color &color) {
	for (unsigned y = 0; y < ROWS; ++y) {
		for (unsigned x = 0; x < COLUMNS; ++x) {
			show(x, y, character, color);
		}
	}
	setPos(0, 0);
}

void GraphicsStream::flush() {
	print(buffer, pos);
	pos = 0;
}

void GraphicsStream::draw() {
	flush();
	Point pos = START;
	for (unsigned y = 0; y < ROWS; y++) {
		for (unsigned x = 0; x < COLUMNS; x++) {
			Cell &c = cell[((offset + y) % ROWS) * COLUMNS + x];
			graphics.text(pos, &c.character, 1, c.color, FONT);
			pos.x += FONT->width;
		}
		pos.x = START.x;
		pos.y += FONT->height;
	}
}
