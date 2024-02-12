/*! \file
 *  \brief Contains a walking cat.
 */
#pragma once

#include "utils/png.h"

class Cat {
	const unsigned animationCount = 12;
	const unsigned height = 199;
	const unsigned width = 392;
	int x, y;
	unsigned animationCurrent;
	int position;
	PNG image;

 public:
	explicit Cat(const char * image = "cat.png");
	void attract();
	void walk();
};
