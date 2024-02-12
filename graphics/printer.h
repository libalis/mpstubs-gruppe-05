/*! \file
 *  \brief \ref GraphicsPrinter and its \ref AbstractGraphicsPrinter "abstraction"
 */

#pragma once

#include "graphics/framebuffer.h"
#include "graphics/primitives.h"
#include "graphics/fonts/font.h"
#include "utils/math.h"
#include "utils/png.h"

/*! \brief Abstraction of basic graphics printing functions
 * \ingroup gfx
 *
 * The actual implementation is placed in the inherited template class
 * \ref GraphicsPrinter for performance reasons.
 */
class AbstractGraphicsPrinter {
 protected:
	/*! \brief Check if a printer is available for a video mode
	 *
	 * This is required since printers are defined during compile time for
	 * performance reasons.
	 *
	 * \tparam colordepth color depth of video mode
	 * \tparam offset_red Bit position of red color mask in video mode
	 * \tparam offset_green Bit position of green color mask in video mode
	 * \tparam offset_blue Bit position of blue color mask in video mode
	 * \tparam bits_red Size of red color mask in video mode
	 * \tparam bits_green Size of green color mask in video mode
	 * \tparam bits_blue Size of blue color mask in video mode
	 * \return `true` if a printer for the video mode is available
	 */
	virtual bool checkMode(uint8_t colordepth, uint8_t offset_red, uint8_t offset_green, uint8_t offset_blue,
	                        uint8_t bits_red, uint8_t bits_green, uint8_t bits_blue) = 0;

 public:
	/*! \brief Initialize printer with actual screen dimensions
	 *
	 * \param width visible width of graphics screen
	 * \param height visible height of graphics screen
	 * \param pitch width of graphics screen (including invisible part, has to be at least `width`)
	 */
	virtual void init(unsigned width, unsigned height, unsigned pitch) = 0;

	/*! \brief Set the video memory address
	 *
	 * \param lfb pointer to the linear framebuffer (lfb)
	 */
	virtual void buffer(void* lfb) = 0;

	/*! \brief Clear all pixel of the current back buffer
	 * (set full screen to black)
	 */
	virtual void clear() = 0;

	/*! \brief Check if a \ref Point can be displayed at the current resolution
	 *
	 * \param p Coordinates to check
	 * \return 'true' if can be displayed
	 */
	virtual bool valid(const Point& p) const = 0;

	/*! \brief Number of vertical pixels in current resolution
	 *
	 * \return Height of the screen in current video mode
	 */
	virtual unsigned height() const = 0;

	/*! \brief Number of horizontal pixels in current resolution
	 *
	 * \return Width of the screen in current video mode
	 */
	virtual unsigned width() const = 0;

	/*! \brief Draw a pixel
	 *
	 * \param p Coordinates of the pixel
	 * \param color Color of the pixel
	 */
	virtual void pixel(const Point& p, const Color& color) = 0;

	/// \copydoc AbstractGraphicsPrinter::pixel()
	virtual void pixel(const Point& p, const ColorAlpha& color) = 0;

	/*! \brief Draw a line
	 *
	 * \param start Coordinates of the begin of the line
	 * \param end Coordinates of the end of the line
	 * \param color Color of the line
	 */
	virtual void line(const Point& start, const Point& end, const Color& color) = 0;

	/// \copydoc AbstractGraphicsPrinter::line()
	virtual void line(const Point& start, const Point& end, const ColorAlpha& color) = 0;

	/*! \brief Draw a rectangle on the current back buffer
	 *
	 * \param start Coordinate of the rectangles upper left corner
	 * \param end Coordinate of the rectangles lower right corner
	 * \param color Color of the rectangle
	 * \param filled If set, the rectangle will be filled with the same color.
	 *                (otherwise only borders will be drawn)
	 */
	virtual void rectangle(const Point& start, const Point& end, const Color& color, bool filled = true) = 0;

	/// \copydoc AbstractGraphicsPrinter::rectangle()
	virtual void rectangle(const Point& start, const Point& end, const ColorAlpha& color, bool filled = true) = 0;

	/*! \brief Change the current font for text output in video mode
	 *
	 * \param new_font Font to be used on subsequent calls to
	 * \ref AbstractGraphicsPrinter::text() (without explicit font parameter)
	 */
	virtual void font(const Font& new_font) = 0;

	/*! \brief Print text (without automatic word wrap).
	 *
	 * \param p Upper left start position of the text
	 * \param string Pointer to char array containing the text to be displayed
	 * \param len Number of characters to be displayed
	 * \param color Color for the text characters
	 * \param font Explicit font -- or `nullptr` to use default font (set by \ref font method)
	 */
	virtual void text(const Point& p, const char* string, unsigned len, const Color& color,
	                  const Font * font = nullptr) = 0;

	/// \copydoc AbstractGraphicsPrinter::text()
	virtual void text(const Point& p, const char* string, unsigned len, const ColorAlpha& color,
	                  const Font * font = nullptr) = 0;

	/*! \brief Draw a \ref PNG image (or detail)
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
	virtual void image(const Point &p, PNG &image, unsigned width = 0, unsigned height = 0,
	                   unsigned offset_x = 0, unsigned offset_y = 0) = 0;

	/*! \brief Draw a GIMP image (or detail)
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
	virtual void image(const Point &p, const GIMP &image, unsigned width = 0, unsigned height = 0,
	                   unsigned offset_x = 0, unsigned offset_y = 0) = 0;

	/*! \brief Draw a sprite.
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
	virtual void image(const Point& p, const Color * image,  unsigned width, unsigned height,
	                   unsigned offset_x = 0, unsigned offset_y = 0) = 0;

	/*! \brief Draw a sprite with alpha blending (transparency).
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
	virtual void image(const Point& p, const ColorAlpha * image,  unsigned width, unsigned height,
	                   unsigned offset_x = 0, unsigned offset_y = 0) = 0;

	static AbstractGraphicsPrinter* getMode(uint8_t colordepth, uint8_t offset_red, uint8_t offset_green,
	                                        uint8_t offset_blue, uint8_t bits_red, uint8_t bits_green, uint8_t bits_blue);
};

/*! \brief Actual implementation of basic graphics printing functions
 * \ingroup gfx
 *
 * The implementation as template class requires the definition of the
 * desired video mode during compile time (which is required anyways
 * since the video mode is set in the \ref Multiboot headers).
 * Hence, the compiler is able to optimize the (intensively used) code for the
 * actual color bit masks, which results in high performance gain.
 *
 * \tparam COLORDEPTH color depth of video mode
 * \tparam OFFSET_RED Bit position of red color mask in video mode
 * \tparam OFFSET_GREEN Bit position of green color mask in video mode
 * \tparam OFFSET_BLUE Bit position of blue color mask in video mode
 * \tparam BITS_RED Size of red color mask in video mode
 * \tparam BITS_GREEN Size of green color mask in video mode
 * \tparam BITS_BLUE Size of blue color mask in video mode
 */
template <unsigned COLORDEPTH,
          uint8_t OFFSET_RED, uint8_t OFFSET_GREEN, uint8_t OFFSET_BLUE,
          uint8_t BITS_RED, uint8_t BITS_GREEN, uint8_t BITS_BLUE>
class GraphicsPrinter : public Framebuffer<COLORDEPTH,
                                           OFFSET_RED, OFFSET_GREEN, OFFSET_BLUE,
                                           BITS_RED, BITS_GREEN, BITS_BLUE>, public AbstractGraphicsPrinter {
	typedef Framebuffer<COLORDEPTH, OFFSET_RED, OFFSET_GREEN, OFFSET_BLUE,
	                    BITS_RED, BITS_GREEN, BITS_BLUE> FramebufferBase;
	typedef typename FramebufferBase::Pixel Pixel;

	/*! \brief currently active font
	 */
	Font const * active_font;

	/*! \brief Generic helper function to draw a sprite image (or detail)
	 *
	 * \param p Coordinate of the images upper left corner
	 * \param image Source image to display
	 * \param width Width of the image detail
	 * \param height Height of the image detail
	 * image_width
	 * \param offset_x Right offset of the source image
	 * \param offset_y Top offset of the source image
	 */
	template <enum SpriteColorMode COLOR, bool ALPHA, unsigned BITDEPTH>
	void sprite(Point p, const struct SpritePixel<COLOR, ALPHA, BITDEPTH> * image,
		        unsigned width, unsigned height, unsigned image_width, unsigned offset_x = 0, unsigned offset_y = 0) {
		if (p.x < 0) {
			offset_x -= p.x;
			if (offset_x > width || static_cast<int>(width) + p.x < 0) {
				return;
			}
			width += p.x;
			p.x = 0;
		}

		if (p.y < 0) {
			offset_y -= p.y;
			if (offset_y > height || static_cast<int>(height) + p.y < 0) {
				return;
			}
			height += p.y;
			p.y = 0;
		}

		if (p.x >= static_cast<int>(this->screen_width) || p.y >= static_cast<int>(this->screen_height)) {
			return;
		}

		if (p.x + width >= this->screen_width) {
			width = this->screen_width - p.x;
		}
		if (p.y + height >= this->screen_height) {
			height = this->screen_height - p.y;
		}

		for (unsigned y = offset_y; y < offset_y + height; ++y) {
			Pixel * pos = this->get(p);
			for (unsigned x = offset_x; x < offset_x + width; ++x) {
				*pos = image[y*image_width + x];
				pos++;
			}
			p.y += 1;
		}
	}

	/// \copydoc AbstractGraphicsPrinter::pixel()
	template <enum SpriteColorMode COLOR, bool ALPHA, unsigned BITS>
	void pixel(const Point &p, const SpritePixel<COLOR, ALPHA, BITS>& color) {
		if (valid(p)) {
			this->set(p, color);
		}
	}

	/// \copydoc AbstractGraphicsPrinter::line()
	template <enum SpriteColorMode COLOR, bool ALPHA, unsigned BITS>
	void line(const Point& start, const Point& end, const SpritePixel<COLOR, ALPHA, BITS>& color) {
		const int d_x = Math::abs(end.x - start.x);
		const int d_y = Math::abs(end.y - start.y);
		const int steps = d_x < d_y ? (d_y + 1) : (d_x + 1);
		int D = 2 * (d_x < d_y ? (d_x - d_y) : (d_y - d_x));
		const int DE = d_x < d_y ? (d_x << 1) : (d_y << 1);
		const int DNE = (d_x < d_y ? (d_x - d_y) : (d_y - d_x)) << 1;
		int x_i1 = d_x < d_y ? 0 : 1;
		int x_i2 = 1;
		int y_i1 = d_x < d_y ? 1 : 0;
		int y_i2 = 1;
		if (start.x > end.x) {
			x_i1 = -x_i1;
			x_i2 = -x_i2;
		}
		if (start.y > end.y) {
			y_i1 = -y_i1;
			y_i2 = -y_i2;
		}
		int x = start.x;
		int y = start.y;
		for (int i = 0; i < steps; i++) {
			if (x >= 0 && y >= 0 && x < static_cast<int>(this->screen_width) && y < static_cast<int>(this->screen_height)) {
				this->set(x, y, color);
			}
			if (D < 0) {
				D += DE;
				x += x_i1;
				y += y_i1;
			} else {
				D += DNE;
				x += x_i2;
				y += y_i2;
			}
		}
	}

	/// \copydoc AbstractGraphicsPrinter::rectangle()
	template <enum SpriteColorMode COLOR, bool ALPHA, unsigned BITS>
	void rectangle(const Point& start, const Point& end, const SpritePixel<COLOR, ALPHA, BITS>& color, bool filled) {
		const int w = width();
		const int h = height();
		const int fromX = Math::max(0, Math::min(start.x, end.x));
		const int fromY = Math::max(0, Math::min(start.y, end.y));
		const int toX = Math::min(w - 1, Math::max(start.x, end.x));
		const int toY = Math::min(h - 1, Math::max(start.y, end.y));
		if (toX < 0 || toY < 0 || fromX >= w || fromY >= h) {
			return;
		}
		if (filled) {
			Point line_start(fromX, fromY);
			for (int y = fromY; y < toY; ++y) {
				Pixel* pos = this->get(line_start);
				for (int x = fromX; x < toX; ++x) {
					*pos = color;
					pos++;
				}
				line_start.y++;
			}
		} else {
			line(Point(fromX, fromY), Point(fromX, toY - 1), color);
			line(Point(fromX + 1, fromY), Point(toX - 1, fromY), color);
			line(Point(fromX, toY), Point(toX - 1, toY), color);
			line(Point(toX, fromY), Point(toX, toY), color);
		}
	}

	/*! \brief Helper function to draw a font pixel image detail
	 *
	 * \see \ref text
	 *
	 * \param p Coordinate of the images upper left corner
	 * \param bitmap Font character bitmap source to display
	 * \param width Width of the font
	 * \param height Height of the font
	 * \param color Color for the character
	 */
	template <enum SpriteColorMode COLOR, bool ALPHA, unsigned BITS>
	void bitmap(const Point& p, const void* bitmap, const unsigned width, const unsigned height,
	            const SpritePixel<COLOR, ALPHA, BITS>& color) {
		unsigned short width_byte = width / 8 + ((width % 8 != 0) ? 1 : 0);
		const char* sprite = reinterpret_cast<const char*>(bitmap);
		for (unsigned y = 0; y < height; ++y) {
			Pixel * pixel = this->get(p) + y * this->screen_width;
			for (unsigned x = 0; x < width_byte; ++x) {
				for (int src = 7; src >= 0; --src) {
					if ((1 << src) & *sprite) {
						*pixel = color;
					}
					pixel++;
				}
				sprite++;
			}
		}
	}

	/*! \brief Helper function to print text
	 *
	 * \param p Upper left start position of the text
	 * \param string Pointer to char array containing the text to be displayed
	 * \param len Number of characters to be displayed
	 * \param color Color for the text characters
	 * \param font Explicit font -- or `nullptr` to use default font (set by \ref font method)
	 */
	template <enum SpriteColorMode COLOR, bool ALPHA, unsigned BITS>
	void text(const Point& p, const char* string, unsigned len,
	          const SpritePixel<COLOR, ALPHA, BITS>& color, const Font * font) {
		if (font == nullptr) {
			font = active_font;
		}
		if (font != nullptr) {
			Point pos = p;
			for(unsigned i = 0; i < len; ++i) {
				this->bitmap(pos, font->symbol(string[i]), font->width,  font->height, color);
				pos.x += font->width;
				if (pos.x + font->width > this->screen_width) {
					pos.x = 0;
					pos.y += font->height;
				}
			}
		}
	}

	/*! \brief Check if a \ref Point can be displayed at the current resolution
	 *
	 * \param x X position to check
	 * \param y Y position to check
	 * \return 'true' if can be displayed
	 */
	bool valid(const int x, const int y) const {
		return x >= 0 &&
		       y >= 0 &&
		       static_cast<unsigned>(x) < this->screen_width &&
		       static_cast<unsigned>(y) < this->screen_height;
	}

 protected:
	/// \copydoc AbstractGraphicsPrinter::checkMode()
	bool checkMode(uint8_t required_COLORDEPTH,
	               uint8_t required_red_offset, uint8_t required_green_offset, uint8_t required_blue_offset,
	               uint8_t required_red_size, uint8_t required_green_size, uint8_t required_blue_size) {
		return COLORDEPTH == required_COLORDEPTH && OFFSET_RED == required_red_offset &&
		       OFFSET_GREEN == required_green_offset && OFFSET_BLUE == required_blue_offset &&
		       BITS_RED == required_red_size && BITS_GREEN == required_green_size && BITS_BLUE == required_blue_size;
	}

 public:
	/*! \brief Constructor
	 */
	GraphicsPrinter() {}

	/// \copydoc AbstractGraphicsPrinter::init()
	void init(unsigned width, unsigned height, unsigned pitch) {
		FramebufferBase::init(width, height, pitch);
		active_font = Font::get("Sun", 12, 22);
	}

	/// \copydoc AbstractGraphicsPrinter::buffer()
	void buffer(void* lfb) {
		FramebufferBase::buffer(lfb);
	}

	/// \copydoc AbstractGraphicsPrinter::clear()
	void clear() {
		FramebufferBase::clear();
	}

	/// \copydoc AbstractGraphicsPrinter::valid()
	bool valid(const Point &p) const {
		return valid(p.x, p.y);
	}

	/// \copydoc AbstractGraphicsPrinter::height()
	unsigned height() const {
		return this->screen_height;
	}

	/// \copydoc AbstractGraphicsPrinter::width()
	unsigned width() const {
		return this->screen_width;
	}

	/// \copydoc AbstractGraphicsPrinter::pixel()
	void pixel(const Point &p, const Color& color) {
		return pixel<RGB, false, 8>(p, color);
	}

	/// \copydoc AbstractGraphicsPrinter::pixel()
	void pixel(const Point &p, const ColorAlpha& color) {
		return pixel<RGB, true, 8>(p, color);
	}

	/// \copydoc AbstractGraphicsPrinter::line()
	void line(const Point& start, const Point& end, const Color& color) {
		return line<RGB, false, 8>(start, end, color);
	}

	/// \copydoc AbstractGraphicsPrinter::line()
	void line(const Point& start, const Point& end, const ColorAlpha& color) {
		return line<RGB, true, 8>(start, end, color);
	}

	/// \copydoc AbstractGraphicsPrinter::rectangle()
	void rectangle(const Point& start, const Point& end, const Color& color, bool filled) {
		return rectangle<RGB, false, 8>(start, end, color, filled);
	}

	/// \copydoc AbstractGraphicsPrinter::rectangle()
	void rectangle(const Point& start, const Point& end, const ColorAlpha& color, bool filled) {
		return rectangle<RGB, true, 8>(start, end, color, filled);
	}

	/// \copydoc AbstractGraphicsPrinter::font()
	void font(const Font& new_font) {
		active_font = &new_font;
	}

	/// \copydoc AbstractGraphicsPrinter::text()
	void text(const Point& p, const char* string, unsigned len, const Color& color, const Font * font) {
		return text<RGB, false, 8>(p, string, len, color, font);
	}

	/// \copydoc AbstractGraphicsPrinter::text()
	void text(const Point& p, const char* string, unsigned len, const ColorAlpha& color, const Font * font) {
		return text<RGB, true, 8>(p, string, len, color, font);
	}

	/// \copydoc AbstractGraphicsPrinter::image(const Point&,PNG&,unsigned,unsigned,unsigned,unsigned)
	void image(const Point& p, PNG &image, unsigned width = 0, unsigned height = 0,
	           unsigned offset_x = 0, unsigned offset_y = 0) {
		unsigned w = image.get_width();
		unsigned h = image.get_height();
		if (width == 0 || offset_x + width > w) {
			if (offset_x > w) {
				return;
			} else {
				width = w - offset_x;
			}
		}
		if (height == 0 || offset_y + height > h) {
			if (offset_y > h) {
				return;
			} else {
				height = h - offset_y;
			}
		}

		switch (image.get_format()) {
			case PNG::RGB8:
				sprite<RGB, false, 8>(p, reinterpret_cast<const struct SpritePixel<RGB, false, 8> *>(image.get_buffer()),
				                      width, height, w, offset_x, offset_y);
				break;
			case PNG::RGB16:
				sprite<RGB, false, 16>(p, reinterpret_cast<const struct SpritePixel<RGB, false, 16> *>(image.get_buffer()),
				                       width, height, w, offset_x, offset_y);
				break;
			case PNG::RGBA8:
				sprite<RGB, true, 8>(p, reinterpret_cast<const struct SpritePixel<RGB, true, 8> *>(image.get_buffer()),
				                     width, height, w, offset_x, offset_y);
				break;
			case PNG::RGBA16:
				sprite<RGB, true, 16>(p, reinterpret_cast<const struct SpritePixel<RGB, true, 16> *>(image.get_buffer()),
				                      width, height, w, offset_x, offset_y);
				break;
			case PNG::LUMINANCE8:
				sprite<GREYSCALE, false, 8>(p,
				                            reinterpret_cast<const struct SpritePixel<GREYSCALE, false, 8> *>(image.get_buffer()),
				                            width, height, w, offset_x, offset_y);
				break;
			case PNG::LUMINANCE_ALPHA4:
				sprite<GREYSCALE, true, 4>(p, reinterpret_cast<const struct SpritePixel<GREYSCALE, true, 4> *>(image.get_buffer()),
				                           width, height, w, offset_x, offset_y);
				break;
			case PNG::LUMINANCE_ALPHA8:
				sprite<GREYSCALE, true, 8>(p, reinterpret_cast<const struct SpritePixel<GREYSCALE, true, 8> *>(image.get_buffer()),
				                           width, height, w, offset_x, offset_y);
				break;
			default:
				// "Unsupported PNG format";
				break;
		}
	}

	/// \copydoc AbstractGraphicsPrinter::image(const Point&,const GIMP&,unsigned,unsigned,unsigned,unsigned)
	void image(const Point& p, const GIMP &image, unsigned width = 0, unsigned height = 0,
	           unsigned offset_x = 0, unsigned offset_y = 0) {
		unsigned w = image.width;
		unsigned h = image.height;
		if (width == 0 || offset_x + width > w) {
			if (offset_x > w) {
				return;
			} else {
				width = w - offset_x;
			}
		}
		if (height == 0 || offset_y + height > h) {
			if (offset_y > h) {
				return;
			} else {
				height = h - offset_y;
			}
		}

		switch(image.bytes_per_pixel) {
			case 2:  // RGB16
				sprite<RGB, false, 16>(p, reinterpret_cast<const struct SpritePixel<RGB, false, 16> *>(image.pixel_data),
				                       width, height, w, offset_x, offset_y);
				break;
			case 3:  // RGB
				sprite<RGB, false, 8>(p, reinterpret_cast<const struct SpritePixel<RGB, false, 8> *>(image.pixel_data),
				                      width, height, w, offset_x, offset_y);
				break;
			case 4:  // RGBA
				sprite<RGB, true, 8>(p, reinterpret_cast<const struct SpritePixel<RGB, true, 8> *>(image.pixel_data),
				                     width, height, w, offset_x, offset_y);
				break;
			default:
				// "Unsupported PNG format";
				break;
		}
	}

	/// \copydoc AbstractGraphicsPrinter::image(const Point&,const Color*,unsigned,unsigned,unsigned,unsigned)
	void image(const Point& p, const Color * image, unsigned width, unsigned height,
	           unsigned offset_x = 0, unsigned offset_y = 0) {
		sprite<RGB, false, 8>(p, image, width, height, width, offset_x, offset_y);
	}

	/// \copydoc AbstractGraphicsPrinter::image(const Point&,const ColorAlpha*,unsigned,unsigned,unsigned,unsigned)
	void image(const Point& p, const ColorAlpha * image, unsigned width, unsigned height,
	           unsigned offset_x = 0, unsigned offset_y = 0) {
		sprite<RGB, true, 8>(p, image, width, height, width,  offset_x, offset_y);
	}
};
