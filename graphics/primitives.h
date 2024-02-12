/*! \file
 *  \brief Graphics primitives like \ref Point
 */

#pragma once

/*! \brief Coordinate on the graphic screen
 * \ingroup gfx
 */
struct Point {
	/*! \brief X Position
	 */
	int x;

	/*! \brief X Position
	 */
	int y;

	/*! \brief Default Constructor
	 *
	 * Initializing the point to the initial position (0, 0)
	 */
	Point() : x(0), y(0) { }

	/*! \brief Constructor
	 *
	 * \brief x X Position
	 * \brief y Y Position
	 */
	Point(int x, int y) : x(x), y(y) { }

	/*! \brief Summation of two points
	 */
	Point operator+(const Point& that) const {
		return Point(x + that.x, y + that.y);
	}

	/*! \brief Assignment summation of two points
	 */
	Point& operator+=(const Point& that) {
		x += that.x;
		y += that.y;
		return *this;
	}

	/*! \brief Difference of two points
	 */
	Point operator-(const Point& that) const {
		return Point(x - that.x, y - that.y);
	}

	/*! \brief Assignment difference of two points
	 */
	Point& operator-=(const Point& that){
		x -= that.x;
		y -= that.y;
		return *this;
	}
};

/*! \brief Color modes
 * \ingroup gfx
 */
enum SpriteColorMode {
	RGB,        ///< Additive color mode (red, green & blue)
	GREYSCALE,  ///< Greyscale
};

/*! \brief Sprite pixel component
 * \ingroup gfx
 *
 * \tparam BITS mask size
 */
template <unsigned BITS> struct __attribute__((packed)) SpritePixelComponent {
	/*! \brief Sprite pixel component value
	 */
	unsigned int value : BITS;

	/*! \brief Default constructor
	 * (sets initial value to zero)
	 */
	SpritePixelComponent() : value(0) {}

	/*! \brief Constructor
	 *
	 * \param value Value for component
	 */
	explicit SpritePixelComponent(unsigned int value) : value(value) {}
};

template <enum SpriteColorMode COLOR, bool ALPHA, unsigned BITS> struct __attribute__((packed)) SpritePixel;

/*! \brief Colored pixel with transparency
 * \ingroup gfx
 *
 * \tparam BITS Size of mask
 */
template <unsigned BITS> struct __attribute__((packed)) SpritePixel<RGB, true, BITS> {
	SpritePixelComponent<BITS> red;
	SpritePixelComponent<BITS> green;
	SpritePixelComponent<BITS> blue;
	SpritePixelComponent<BITS> alpha;
	SpritePixel<RGB, true, BITS>(unsigned red, unsigned green, unsigned blue, unsigned alpha)
	       : red(red), green(green), blue(blue), alpha(alpha) {}
	SpritePixel<RGB, true, BITS>() {}
};

/*! \brief Colored pixel without transparency
 * \ingroup gfx
 *
 * \tparam BITS Size of mask
 */
template <unsigned BITS> struct  __attribute__((packed)) SpritePixel<RGB, false, BITS> {
	SpritePixelComponent<BITS> red;
	SpritePixelComponent<BITS> green;
	SpritePixelComponent<BITS> blue;

	SpritePixel<RGB, false, BITS>(unsigned red, unsigned green, unsigned blue) : red(red), green(green), blue(blue) {}
	SpritePixel<RGB, false, BITS>() {}
};

/*! \brief Greyscale pixel with transparency
 * \ingroup gfx
 *
 * \tparam BITS Size of mask
 */
template <unsigned BITS> struct __attribute__((packed)) SpritePixel<GREYSCALE, true, BITS> {
	SpritePixelComponent<BITS> luminance;
	SpritePixelComponent<BITS> alpha;
	SpritePixel<GREYSCALE, true, BITS>(unsigned luminance, unsigned alpha) : luminance(luminance), alpha(alpha) { }
	SpritePixel<GREYSCALE, true, BITS>(){}
};

/*! \brief Greyscale pixel without transparency
 * \ingroup gfx
 *
 * \tparam BITS Size of mask
 */
template <unsigned BITS> struct  __attribute__((packed)) SpritePixel<GREYSCALE, false, BITS> {
	SpritePixelComponent<BITS> luminance;

	explicit SpritePixel<GREYSCALE, false, BITS>(unsigned luminance) : luminance(luminance) { }
	SpritePixel<GREYSCALE, false, BITS>(){}
};

typedef struct SpritePixel<RGB, false, 8> Color;
typedef struct SpritePixel<RGB, true, 8> ColorAlpha;

/*! \brief GIMP image
 * \ingroup gfx
 *
 * Image exported as C-source (without `Glib` types!) in [GIMP](https://www.gimp.org/),
 * supports alpha blending (transparency).
 */
struct GIMP {
	unsigned int width;
	unsigned int height;
	unsigned int bytes_per_pixel;
	unsigned char pixel_data[];
};
