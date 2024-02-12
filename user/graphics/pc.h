/*! \file
 *  \brief Prints a small PC that counts the FPS
 */

#pragma once

#include "thread/thread.h"

#include "device/graphicsstream.h"

#include "utils/png.h"

class PC : public Thread {
	PNG image;
	GraphicsStream gout;

 public:
	explicit PC(const char * image = "pc.png");

	/*! \brief Print the PC
	 */
	void boot();

	void action();
};
