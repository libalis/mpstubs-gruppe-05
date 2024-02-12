/*! \file
 *  \brief Fake Pong
 */

#pragma once

#include "utils/random.h"
#include "graphics/primitives.h"

class Pong {
	int height;
	int width;
	Point pos, dir;
	const Point paddle;
	static const int size = 100;
	static const int offset = 10;

	Random rand;

 public:
	Pong() : height(570), width(1023), pos(width/2, height/2), dir(1, 1), paddle(20, size), rand(13) {}

	void start();
	void play();
};
