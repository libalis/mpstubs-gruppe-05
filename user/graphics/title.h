/*! \file
 *  \brief Prints text (from a file)
 */

#pragma once

#include "graphics/primitives.h"
#include "graphics/fonts/font.h"

class Title {
	Color color;
	Font * font;
	char buffer[1024];
	int len;

 public:
	explicit Title(const char * file = "title.txt", const Color &color = Color(0x33, 0x88, 0xcc));

	void print(const Point &p = Point(230, 10));
};
