/*! \file
 *  \brief Prints the fire animation, adopted from http://lodev.org/cgtutor/fire.html
 */

#pragma once

#include "graphics/primitives.h"
#include "thread/thread.h"

class Fire : public Thread {
	unsigned height;
	unsigned width;

	Point offset;

 public:
	static const unsigned max_height = 400;
	static const unsigned max_width = 1920;

	Fire();

	virtual void action();

	void ignite();
	void burn();
};
