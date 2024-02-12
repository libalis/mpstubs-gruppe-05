/*! \file
 *  \brief \ref GuardedGraphics, a \ref Guarded "guarded" interface for \ref Graphics
 */

#pragma once

#include "device/graphics.h"
#include "interrupt/guarded.h"

/*! \brief \ref Guarded interface to \ref Graphics used by user applications.
 */
class GuardedGraphics : public Graphics {
 public:
	/*! \brief The constructor passes the pointers to the base-class
	 *  constructor.
	 */
	GuardedGraphics(unsigned size, void* frontbuffer, void* backbuffer) : Graphics(size, frontbuffer, backbuffer) {}

	/*! \copydoc Graphics::switchBuffers()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref Graphics, with the only difference that the call will be
	 *       protected by a \ref Guarded object.
	 *
	 *  \opt Extend method by a \ref Guarded object.
	 */
	bool switchBuffers() {

		return Graphics::switchBuffers();
	}

	/*! \copydoc Graphics::scanoutFrontbuffer()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref Graphics, with the only difference that the call will be
	 *       protected by a \ref Guarded object.
	 *
	 *  \opt Extend method by a \ref Guarded object.
	 */
	void scanoutFrontbuffer() {

		Graphics::scanoutFrontbuffer();
	}
};
