/*! \file
 *  \brief Prints a small PC that counts the FPS
 */

#pragma once

#include "thread/thread.h"

#include "device/graphicsstream.h"

class FPS : public Thread {
	volatile unsigned count_drawn;
	volatile unsigned count_undrawn;

	GraphicsStream gout;

 public:
	FPS();

	/*! \brief Prints the FPS
	 */
	void show();

	/*! \brief FPS counting function
	 * \param drawn Specifies whether the frame was written to video memory
	 */
	void frame(bool drawn);

	void action();
};
