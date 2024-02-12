/*! \file
 *  \brief \ref Framebuffer implementing primitive graphic operations
 */

#pragma once

#include "types.h"
#include "graphics/primitives.h"
#include "utils/string.h"

/*! \brief Implementation of primitive operations on a memory area used as framebuffer.
 * \ingroup gfx
 *
 * The implementation as template class allows the compiler to heavily optimize
 * the bit operations depending on the video mode.
 *
 * \tparam COLORDEPTH color depth of video mode
 * \tparam OFFSET_RED Bit position of red color mask in video mode
 * \tparam OFFSET_GREEN Bit position of green color mask in video mode
 * \tparam OFFSET_BLUE Bit position of blue color mask in video mode
 * \tparam BITS_RED Size of red color mask in video mode
 * \tparam BITS_GREEN Size of green color mask in video mode
 * \tparam BITS_BLUE Size of blue color mask in video mode
 */
template <unsigned char COLORDEPTH, unsigned char OFFSET_RED, unsigned char OFFSET_GREEN,
          unsigned char OFFSET_BLUE, unsigned char BITS_RED, unsigned char BITS_GREEN,
          unsigned char BITS_BLUE>
class Framebuffer {
	/*! \brief Start address of the linear framebuffer
	 */
	uintptr_t framebuffer;

	/*! \brief Internal width of the screen
	 *
	 * At least the visible width of the screen, depends on the hardware but
	 * required for calculating the rows
	 */
	unsigned pitch;

 protected:
	/*! \brief Visible width of the screen
	 */
	unsigned screen_width;

	/*! \brief Visible height of the screen
	 */
	unsigned screen_height;

	/*! \brief Initialize screen dimensions
	 *
	 * \param width visible width of graphics screen
	 * \param height visible height of graphics screen
	 * \param pitch width of graphics screen (including invisible part, has to be at least `width`)
	 */
	void init(const unsigned width, const unsigned height, const unsigned pitch) {
		this->screen_width = width;
		this->screen_height = height;
		this->pitch = pitch;
	}

	/*! \brief Set the video memory address
	 *
	 * \param lfb pointer to the linear framebuffer (lfb)
	 */
	void buffer(void* lfb) {
		framebuffer = reinterpret_cast<uintptr_t>(lfb);
	}

	/*! \brief Clear all pixel of the current back buffer
	 * (set full screen to black)
	 */
	void clear() {
		memset(reinterpret_cast<void*>(framebuffer), 0, screen_height * pitch);
	}

	/*! \brief Pixel component
	 *
	 * \tparam OFFSET Bit position of mask
	 * \tparam BITS Size of mask
	 */
	template <unsigned OFFSET, unsigned SIZE>
	class PixelComponent{
		unsigned : OFFSET;      ///< Reserved space for offset
		unsigned value : SIZE;  ///< Value

	public:
		/*! \brief Constructor
		 *
		 * \param value Initial component value
		 */
		explicit PixelComponent(unsigned value) : value(value) {}

		/*! \brief Assign component value
		 * (from a \ref SpritePixelComponent with different bit mask size)
		 *
		 * \tparam BITS Size of bit mask
		 * \param other new component value
		 */
		template<unsigned BITS>
		void set(const struct SpritePixelComponent<BITS> &other) {
			value = BITS > SIZE ? (other.value >> (BITS - SIZE)) : (other.value << (SIZE - BITS));
		}

		/*! \brief Assign component value
		 * (from a \ref SpritePixelComponent with same bit mask size)
		 *
		 * \param other new component value
		 */
		void set(const struct SpritePixelComponent<SIZE> &other) {
			value = other.value;
		}

		/*! \brief Assign component value
		 * (from an integer)
		 *
		 * \param value new component value
		 */
		void set(unsigned value) {
			value = 8 > SIZE ? (value >> (8 - SIZE)) : (value << (SIZE - 8));
		}

		/*! \brief Alpha blend component value
		 * (from a \ref SpritePixelComponent with different bit mask size)
		 *
		 * \tparam BITS Size of bit mask
		 * \param other component value to blend
		 * \param alpha transparency used for blending
		 */
		template<unsigned BITS>
		void blend(const struct SpritePixelComponent<BITS> &other, const struct SpritePixelComponent<BITS> &alpha) {
			int other_value = BITS > SIZE ? (other.value >> (BITS - SIZE)) : (other.value << (SIZE - BITS));
			int other_alpha = BITS > SIZE ? (alpha.value >> (BITS - SIZE)) : (alpha.value << (SIZE - BITS));
			value += ((other_value - static_cast<int>(value)) * other_alpha) >> SIZE;
		}

		/*! \brief Alpha blend component value
		 * (from a \ref SpritePixelComponent with same bit mask size)
		 *
		 * \param other component value to blend
		 * \param alpha transparency used for blending
		 */
		void blend(const struct SpritePixelComponent<SIZE> &other, const struct SpritePixelComponent<SIZE> &alpha) {
			value += ((static_cast<int>(other.value) - static_cast<int>(value)) * alpha.value) >> SIZE;
		}
	} __attribute__((packed));

	/*! \brief Pixel (colored)
	 */
	union Pixel {
		/*! \brief Bits per pixel
		 */
		struct {
			unsigned data : COLORDEPTH;  ///< RGB value
		} __attribute__((packed));

		PixelComponent<OFFSET_RED,   BITS_RED> red;      ///< Red color component
		PixelComponent<OFFSET_GREEN, BITS_GREEN> green;  ///< Green color component
		PixelComponent<OFFSET_BLUE,  BITS_BLUE> blue;    ///< Blue color component

		/*! \brief Constructor (using RGB value)
		 *
		 * \param data RGB value
		 */
		explicit Pixel(const unsigned data) : data(data) { }

		/*! \brief Constructor (using explicit RGB components)
		 *
		 * Unused bits are zeroed.
		 *
		 * \param r Red color component
		 * \param g Green color component
		 * \param b Blue color component
		 */
		Pixel(const unsigned r, const unsigned g, const unsigned b) : data(0) {
			red.set(r);
			green.set(g);
			blue.set(b);
		}

		/*! \brief Constructor (using \ref SpritePixel)
		 *
		 * \tparam ALPHA `true` if alpha channel
		 * \tparam BITS Size of mask
		 * \param other other
		 */
		template<bool ALPHA, unsigned BITS>
		explicit Pixel(const struct SpritePixel<RGB, ALPHA, BITS> &other) {
			red.set(other.red);
			green.set(other.green);
			blue.set(other.blue);
		}

		/*! \brief Get color of pixel
		 *
		 * \return color of pixel
		 */
		Color getColor() const {
			return Color(red, green, blue);
		}

		/*! \brief Assign pixel (with colored \ref SpritePixel)
		 *
		 * \tparam BITS Size of other pixels mask
		 * \param other other pixel
		 */
		template<unsigned BITS>
		Pixel& operator=(const struct SpritePixel<RGB, false, BITS> &other) {
			red.set(other.red);
			green.set(other.green);
			blue.set(other.blue);
			return *this;
		}

		/*! \brief Assign pixel (with greyscale \ref SpritePixel)
		 *
		 * \tparam BITS Size of other pixels mask
		 * \param other other pixel
		 */
		template<unsigned BITS>
		Pixel& operator=(const struct SpritePixel<GREYSCALE, false, BITS> &other) {
			red.set(other.luminance);
			green.set(other.luminance);
			blue.set(other.luminance);
			return *this;
		}

		/*! \brief Assign pixel (with greyscale \ref SpritePixel supporting transparency)
		 *
		 * \tparam BITS Size of other pixels mask
		 * \param other other pixel
		 */
		template<unsigned BITS>
		Pixel& operator=(const struct SpritePixel<RGB, true, BITS> &other) {
			red.blend(other.red, other.alpha);
			green.blend(other.green, other.alpha);
			blue.blend(other.blue, other.alpha);
			return *this;
		}

		/*! \brief Assign pixel (with greyscale \ref SpritePixel supporting transparency)
		 *
		 * \tparam BITS Size of other pixels mask
		 * \param other other pixel
		 */
		template<unsigned BITS>
		Pixel& operator=(const struct SpritePixel<GREYSCALE, true, BITS> &other) {
			red.blend(other.luminance, other.alpha);
			green.blend(other.luminance, other.alpha);
			blue.blend(other.luminance, other.alpha);
			return *this;
		}
	} __attribute__((packed));
	static_assert(OFFSET_RED + BITS_RED <= COLORDEPTH &&
	              OFFSET_GREEN + BITS_GREEN <= COLORDEPTH &&
	              OFFSET_BLUE + BITS_BLUE <= COLORDEPTH, "color settings invalid!");

	/*! \brief Get pixel at position
	 *
	 * \param x X position
	 * \param y Y position
	 * \return Pointer to pixel
	 */
	Pixel * get(const unsigned x, const unsigned y) const {
		return reinterpret_cast<Pixel*>(framebuffer + y * pitch) + x;
	}

	/*! \brief Get pixel at position
	 *
	 * \param p Coordinate of position
	 * \return Pointer to pixel
	 */
	Pixel * get(const Point& p) const {
		return get(p.x, p.y);
	}

	/*! \brief Assign color to a pixel at a given position
	 *
	 * \tparam COLOR color or greyscale?
	 * \tparam ALPHA with transparency?
	 * \tparam BITS Size of mask
	 * \param x X position
	 * \param y Y position
	 * \param color color to assign
	 */
	template <enum SpriteColorMode COLOR, bool ALPHA, unsigned BITS>
	void set(const unsigned x, const unsigned y, const SpritePixel<COLOR, ALPHA, BITS>& color) {
		Pixel * pos = get(x, y);
		*pos = color;
	}

	/*! \brief Assign color to a pixel at a given position
	 *
	 * \tparam COLOR color or greyscale?
	 * \tparam ALPHA with transparency?
	 * \tparam BITS Size of mask
	 * \param p Coordinate of position
	 * \param color color to assign
	 */
	template <enum SpriteColorMode COLOR, bool ALPHA, unsigned BITS>
	void set(const Point &p, const SpritePixel<COLOR, ALPHA, BITS>& color) {
		set(p.x, p.y, color);
	}
};
