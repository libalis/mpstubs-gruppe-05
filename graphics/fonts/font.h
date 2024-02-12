/*! \file
 *  \brief \ref Font "Monospaced fonts"
 */

#pragma once

#include "types.h"

/*! \brief Monospaced fonts
 * \ingroup gfx
 *
 * Console fonts are extracted from the Linux kernel ([/lib/fonts/](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/lib/fonts)).
 */
class Font {
	/*! \brief Pointer to bitmap font
	 */
	const unsigned char* data;

	/*! \brief Size in memory of bitmap font
	 */
	const size_t size;

 public:
	/*! \brief Name of font
	 */
	const char *name;

	/*! \brief Width of a character
	 */
	const unsigned width;

	/*! \brief Height of a character
	 */
	const unsigned height;

	/*! \brief Constructor for a font
	 *
	 * \param name Name of font
	 * \param width character width
	 * \param height character height
	 * \param data Pointer to bitmap font
	 */
	Font(const char * name, unsigned width, unsigned height, const unsigned char* data)
	   : data(data), size((((width + (8 >> 1)) / 8) * height)),  name(name), width(width), height(height) {}

	/*! \brief Get bitmap address for a given character
	 *
	 *  \param c character (ASCII)
	 *  \return Pointer to bitmap of character
	 */
	const void * symbol(unsigned char c) const {
		return &data[size * c];
	}

	/*! \brief Find font
	 *
	 *  \param name Name of font (or `nullptr` for any)
	 *  \param width Width of a character (or `0` for any)
	 *  \param height Height of a character (or `0` for any)
	 *  \return Pointer to font or `nullptr` if no matching font was found
	 */
	static Font * get(const char* name = nullptr, unsigned width = 0, unsigned height = 0);

	/*! \brief Get the number of available fonts
	 *  \return number of fonts
	 */
	static unsigned number();
};
