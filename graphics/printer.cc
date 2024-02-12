#include "graphics/printer.h"
#include "utils/size.h"
#include "debug/output.h"

// Predefined video modes (should suit the most common ones)
static GraphicsPrinter<32, 16, 8, 0, 8, 8, 8> default_32bit;
static GraphicsPrinter<24, 16, 8, 0, 8, 8, 8> default_24bit;
static GraphicsPrinter<16, 11, 5, 0, 5, 6, 5> default_16bit;
static GraphicsPrinter<15, 10, 5, 0, 5, 5, 5> default_15bit;
static GraphicsPrinter<8, 5, 2, 0, 3, 3, 2> default_8bit;

AbstractGraphicsPrinter * supportedGraphicModes[] = {
	&default_32bit,
	&default_24bit,
	&default_16bit,
	&default_15bit,
	&default_8bit,
};

AbstractGraphicsPrinter * AbstractGraphicsPrinter::getMode(uint8_t colordepth, uint8_t offset_red,
    uint8_t offset_green, uint8_t offset_blue, uint8_t bits_red, uint8_t bits_green, uint8_t bits_blue) {
	for (unsigned m = 0; m < ::size(supportedGraphicModes); m++) {
		if (supportedGraphicModes[m]->checkMode(colordepth, offset_red, offset_green, offset_blue,
		                                         bits_red, bits_green, bits_blue)) {
			return supportedGraphicModes[m];
		}
	}
	// Show error message (DBG should be copied to serial as well for debugging!)
	DBG_VERBOSE << "No GraphicsPrinter<" << static_cast<int>(colordepth) << ", " << static_cast<int>(offset_red) << ", "
	            << static_cast<int>(offset_green) << ", " << static_cast<int>(offset_blue) << ", "
	            << static_cast<int>(bits_red) << ", " << static_cast<int>(bits_green) << ", "
	            << static_cast<int>(bits_blue) << "> instance available - please add!" << endl;
	return nullptr;
}
