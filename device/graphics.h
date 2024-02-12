/*! \file
 *  \brief The \ref Graphics device is an interface for the \ref Framebuffer
 */

#pragma once

/*! \defgroup gfx Graphics
 *
 * Graphical VESA video modes.
 */

#include "graphics/primitives.h"
#include "graphics/printer.h"

/*! \brief Driver managing the video mode and synchronizing its buffer with the
 * \ref AbstractGraphicsPrinter "graphics printer"
 * \ingroup gfx
 *
 * This device detects the current video mode set by the \ref Multiboot
 * compliant boot loader and initializes a suitable \ref GraphicsPrinter.
 *
 * With the methods \ref Graphics::switchBuffers() (to exchange front- and
 * backbuffer) and \ref Graphics::scanoutFrontbuffer() (copying the contents of
 * the frontbuffer into the video memory) it provides some kind of manually
 * [triple buffering](https://en.wikipedia.org/wiki/Multiple_buffering#Triple_buffering).
 *
 * A typical usage is to fully prepare the back buffer before switching it with
 * the front buffer
 * \code{.cpp}
 * graphics.init();
 * while(true) {
 *     // Draw on back buffer
 *     // using the primitives provided by the driver
 *
 *     graphics.switchBuffers();
 * }
 * \endcode
 *
 * The method \ref Graphics::scanoutFrontbuffer() has to be executed either
 * inside the loop (right after \ref Graphics::switchBuffers() in the example
 * above) or at a predefined interval by employing the \ref Watch.
 *
 * \note The driver requires \ref Multiboot to initialize a video mode, which
 * can be configured using the flags in `boot/multiboot/config.inc`.
 */
class Graphics {
	/*! \brief Pointer to a \ref GraphicsPrinter supporting the current video mode
	 */
	AbstractGraphicsPrinter * printer;

	/*! \brief Pointer to the physical address of the video memory (linear frame buffer)
	 */
	void * address;

	/*! \brief Video memory size required for a full screen picture
	 */
	unsigned size;

	/*! \brief Size of the front (or back) buffer (which has to be at least \ref size)
	 */
	unsigned buffer_size;

	/*! \brief Pointer to the two buffers
	 * (used alternately as front and back buffers)
	 */
	void * const buffer[2];

	/*! \brief Index of the current front buffer
	 */
	int scanout_buffer;

	/*! \brief Has the current front buffer already been drawn?
	 */
	bool refresh;

 public:
	/*! \brief Constructor
	 *
	 * \param size Size of each buffer
	 * \param frontbuffer Pointer to a reserved memory area with a sufficient
	 *                    size to transmit a full screen picture to the video
	 *                     memory at the current video mode / resolution.
	 * \param backbuffer Pointer to a reserved memory area with the same size as
	 *                   the front buffer to prepare the picture.
	 */
	Graphics(unsigned size, void* frontbuffer, void* backbuffer);

	/*! \brief Initialize \ref GraphicsPrinter according to the current video mode
	 *
	 * \param force Do not check video attributes for the linear frame buffer
	 *              (required on our test systems due to some strange VBE behaviour)
	 * \return 'true' if a suitable \ref GraphicsPrinter was found for the video mode
	 */
	bool init(bool force = false);

	/*! \brief Switch front and back buffer
	 * (only if front buffer was already copied to video memory)
	 *
	 * \return `true` if buffers have been switched, `false` if previous front
	 *         buffer wasn't yet fully copied to video memory.
	 */
	bool switchBuffers();

	/*! \brief Copy current front buffer to the video memory
	 */
	void scanoutFrontbuffer();

	/*! \brief Clear all pixel of the current back buffer
	 * (set full screen to black)
	 */
	void clear() {
		printer->clear();
	}

	/*! \brief Check if a \ref Point can be displayed at the current resolution
	 *
	 * \param p Coordinates to check
	 * \return 'true' if can be displayed
	 */
	bool valid(const Point& p) {
		return printer->valid(p);
	}

	/*! \brief Number of vertical pixels in current resolution
	 *
	 * \return Height of the screen in current video mode
	 */
	unsigned height() {
		return printer->height();
	}

	/*! \brief Number of horizontal pixels in current resolution
	 *
	 * \return Width of the screen in current video mode
	 */
	unsigned width() {
		return printer->width();
	}

	/*! \brief Draw a pixel on the current back buffer
	 *
	 * \param p Coordinates of the pixel
	 * \param color Color of the pixel
	 */
	void pixel(const Point &p, const Color &color) {
		printer->pixel(p, color);
	}

	/// \copydoc pixel
	void pixel(const Point &p, const ColorAlpha &color) {
		printer->pixel(p, color);
	}

	/*! \brief Draw a line on the current back buffer
	 *
	 * \param start Coordinates of the begin of the line
	 * \param end Coordinates of the end of the line
	 * \param color Color of the line
	 */
	void line(const Point& start, const Point& end, const Color& color) {
		printer->line(start, end, color);
	}

	/// \copydoc line
	void line(const Point& start, const Point& end, const ColorAlpha& color) {
		printer->line(start, end, color);
	}

	/*! \brief Draw a rectangle on the current back buffer
	 *
	 * \param start Coordinate of the rectangles upper left corner
	 * \param end Coordinate of the rectangles lower right corner
	 * \param color Color of the rectangle
	 * \param filled If set, the rectangle will be filled with the same color.
	 *                (otherwise only borders will be drawn)
	 */
	void rectangle(const Point& start, const Point& end, const Color& color, bool filled = true) {
		printer->rectangle(start, end, color, filled);
	}

	/// \copydoc rectangle
	void rectangle(const Point& start, const Point& end, const ColorAlpha& color, bool filled = true) {
		printer->rectangle(start, end, color, filled);
	}

	/*! \brief Change the current font for text output in video mode
	 *
	 * \param new_font Font to be used on subsequent calls to \ref text (without explicit font parameter)
	 */
	void font(const Font& new_font) {
		printer->font(new_font);
	}

	/*! \brief Print text (without automatic word wrap) on the current back buffer
	 *
	 * \param p Upper left start position of the text
	 * \param string Pointer to char array containing the text to be displayed
	 * \param len Number of characters to be displayed
	 * \param color Color for the text characters
	 * \param font Explicit font -- or `nullptr` to use default font (set by \ref font method)
	 */
	void text(const Point& p, const char* string, unsigned len, const Color& color, const Font * font = nullptr) {
		printer->text(p, string, len, color, font);
	}

	/// \copydoc text
	void text(const Point& p, const char* string, unsigned len, const ColorAlpha& color, const Font * font = nullptr) {
		printer->text(p, string, len, color, font);
	}

	/*! \brief Draw a \ref PNG image [detail] on the current back buffer.
	 *
	 * The image can has to be in a supported \ref PNG format.
	 * Alpha blending (transparency) is supported.
	 *
	 * \param p Coordinate of the images upper left corner
	 * \param image Source image to display
	 * \param width Width of the image detail (full image width of the source image if zero/default value)
	 * \param height Height of the image detail (full image height of the source if zero/default value)
	 * \param offset_x Right offset of the source image
	 * \param offset_y Top offset of the source image
	 */
	void image(const Point& p, PNG &image, unsigned width = 0, unsigned height = 0,
	           unsigned offset_x = 0, unsigned offset_y = 0) {
		printer->image(p, image, width, height, offset_x, offset_y);
	}

	/*! \brief Draw a GIMP image [detail] on the current back buffer.
	 *
	 * The image has to be exported as C-source (without `Glib` types!) in
	 * [GIMP](https://www.gimp.org/), alpha blending (transparency) is supported.
	 *
	 * \param p Coordinate of the images upper left corner
	 * \param image Source image to display
	 * \param width Width of the image detail (full image width of the source image if zero/default value)
	 * \param height Height of the image detail (full image height of the source if zero/default value)
	 * \param offset_x Right offset of the source image
	 * \param offset_y Top offset of the source image
	 */
	void image(const Point& p, const GIMP &image, unsigned width = 0, unsigned height = 0,
	           unsigned offset_x = 0, unsigned offset_y = 0) {
		printer->image(p, image, width, height, offset_x, offset_y);
	}

	/*! \brief Draw a sprite on the current back buffer.
	 *
	 * Each element in the source array will be displayed as a single pixel.
	 *
	 * \param p Coordinate of the sprites upper left corner
	 * \param image Source sprite to display
	 * \param width Width of the sprite detail
	 * \param height Height of the sprite detail
	 * \param offset_x Right offset of the source sprite
	 * \param offset_y Top offset of the source sprite
	 */
	void image(const Point& p, const Color * image, unsigned width, unsigned height,
	           unsigned offset_x = 0, unsigned offset_y = 0) {
		printer->image(p, image, width, height, offset_x, offset_y);
	}

	/*! \brief Draw a sprite with alpha blending (transparency) on the current back buffer.
	 *
	 * Each element in the source array will be displayed as a single pixel.
	 *
	 * \param p Coordinate of the sprites upper left corner
	 * \param image Source sprite to display
	 * \param width Width of the sprite detail
	 * \param height Height of the sprite detail
	 * \param offset_x Right offset of the source sprite
	 * \param offset_y Top offset of the source sprite
	 */
	void image(const Point& p, const ColorAlpha * image, unsigned width, unsigned height,
	           unsigned offset_x = 0, unsigned offset_y = 0) {
		printer->image(p, image, width, height, offset_x, offset_y);
	}
};
