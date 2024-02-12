#include "graphics/fonts/font.h"

#include "types.h"
#include "utils/size.h"
#include "utils/string.h"

#include "graphics/fonts/font_6x10.h"
#include "graphics/fonts/font_6x11.h"
#include "graphics/fonts/font_7x14.h"
#include "graphics/fonts/font_8x8.h"
#include "graphics/fonts/font_8x16.h"
#include "graphics/fonts/font_10x18.h"
#include "graphics/fonts/font_acorn_8x8.h"
#include "graphics/fonts/font_mini_4x6.h"
#include "graphics/fonts/font_pearl_8x8.h"
#include "graphics/fonts/font_sun_12x22.h"
#include "graphics/fonts/font_sun_8x16.h"
#include "graphics/fonts/font_ter_16x32.h"

static Font fonts[] = {
	Font("Standard", 6, 10, fontdata_6x10),
	Font("Standard", 7, 14, fontdata_7x14),
	Font("Standard", 8, 8, fontdata_8x8),
	Font("Standard", 8, 16, fontdata_8x16),
	Font("Standard", 10, 18, fontdata_10x18),
	Font("Acorn", 8, 8, acorndata_8x8),
	Font("Mini", 4, 6, fontdata_mini_4x6),
	Font("Pearl", 8, 8, fontdata_pearl_8x8),
	Font("Sun", 12, 22, fontdata_sun_12x22),
	Font("Sun", 8, 16, fontdata_sun_8x16),
	Font("Terminus", 16, 32, fontdata_ter16x32),
};

unsigned Font::number() {
	return ::size(fonts);
}

Font * Font::get(const char* name, const unsigned width, const unsigned height) {
	for (unsigned i = 0 ; i < number(); i++) {
		if ((name == nullptr || strcmp(name, fonts[i].name) == 0) &&
		    (width == 0 || width == fonts[i].width) &&
		    (height == 0 || height == fonts[i].height)) {
			return &fonts[i];
		}
	}
	return nullptr;
}
