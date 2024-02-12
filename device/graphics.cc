#include "device/graphics.h"

#include "boot/multiboot/data.h"

#include "debug/output.h"
#include "utils/string.h"

/*! \brief Mode Information of the *Vesa BIOS Extension*
 *
 * \see Vesa BIOS Extension [ModeInfoBlock struc](vbe3.pdf#page=38)
 */
struct VbeModeInfo {
	enum ModeAttributes : uint16_t {
		SUPPORTED = 1 << 0,  ///< Mode supported by hardware configuration
		TTY       = 1 << 2,  ///< TTY Output functions supported by BIOS
		COLOR     = 1 << 3,  ///< Color mode (otherwise monochrome)
		GRAPHICS  = 1 << 4,  ///< Graphic mode (otherwise text)
		VGA       = 1 << 5,  ///< VGA compatible
		VGA_PAGED = 1 << 6,  ///< VGA compatible windowed memory mode is available
		LFB       = 1 << 7,  ///< Linear frame buffer mode is available
	};
	uint16_t mode_attributes;

	// Window functions (used by all VBE revisions, but ignored here)
	struct __attribute__((packed)) {
		uint8_t attrib_a;
		uint8_t attrib_b;
		uint16_t granularity;
		uint16_t size;
		uint16_t segment_a;
		uint16_t segment_b;
		uint32_t func_ptr;
	} win;

	uint16_t pitch;       ///< Bytes per scan line
	uint16_t width;       ///< Horizontal resolution in pixels (GRAPHICS) or characters
	uint16_t height;      ///< Vertical resolution in pixels (GRAPHICS) or characters

	uint8_t char_width;   ///< Character cell width in pixels (deprecated)
	uint8_t char_height;  ///< Character cell height in pixels (deprecated)
	uint8_t planes;       ///< Number of memory planes

	uint8_t bpp;          ///< Bits per pixel

	uint8_t banks;        ///< Number of banks
	enum MemoryModel : uint8_t {
		TEXT_MODE    = 0,  ///< Text mode
		CGA          = 1,  ///< CGA graphics
		HERCULES     = 2,  ///< Hercules graphics
		PLANAR       = 3,  ///< Planar
		PACKED       = 4,  ///< Packed pixel
		NON_CHAIN_4  = 5,  ///< Non-chain 4, 256 color
		DIRECT_COLOR = 6,  ///< Direct Color
		YUV          = 7   ///< YUV
	} memory_model;       ///< Memory model type
	uint8_t bank_size;    ///< Bank size in KB
	uint8_t image_pages;  ///< Number of images
	uint8_t reserved;     ///< Reserved for page function

	// Direct Color fields (required for DIRECT_COLOR and YUV memory models)
	uint8_t bits_red;      ///< Size of direct color red mask in bits
	uint8_t offset_red;    ///< Bit position of lsb of red mask
	uint8_t bits_green;    ///< Size of direct color green mask in bits
	uint8_t offset_green;  ///< Bit position of lsb of green mask
	uint8_t bits_blue;     ///< Size of direct color blue mask in bits
	uint8_t offset_blue;   ///< Bit position of lsb of blue mask
	uint8_t bits_rsv;      ///< Size of direct color reserved mask in bits
	uint8_t offset_rsv;    ///< Bit position of lsb of reserved mask

	enum DirectColorAttributes : uint8_t {
		DYNAMIC_COLOR_RAMP = 1 << 0,  ///< Programmable (otherwise fixed) color ramp
		USABLE_BITS        = 1 << 1   ///< Bits in reserved mask are usable (otherwise reserved)
	};
	uint8_t directcolor_attributes;  ///< direct color mode attributes

	// Mandatory information for VBE 2.0 and above
	uint32_t address;                  ///< physical address for flat memory frame buffer
	uint32_t offscreen_memory_offset;  ///< reserved
	uint16_t offscreen_memory_size;    ///< reserved
} __attribute__((packed));

Graphics::Graphics(unsigned size, void* frontbuffer, void* backbuffer)
    : buffer_size(size), buffer{frontbuffer, backbuffer}, scanout_buffer(0), refresh(false) {}

bool Graphics::init(bool force) {
	// Most boot loaders support the Vesa BIOS Extension (VBE) information quite well
	Multiboot::VBE * vbe = Multiboot::getVesaBiosExtensionInfo();
	if (vbe != nullptr) {
		// However, you have to manually parse the required data out of all the outdated crap
		struct VbeModeInfo * vbe_info = reinterpret_cast<struct VbeModeInfo *>(static_cast<uintptr_t>(vbe->mode_info));
		// Is there linear frame buffer (or just ignore the check with `force` due to a misleading information)?
		if (force || (vbe_info->mode_attributes & VbeModeInfo::ModeAttributes::LFB) != 0) {
			// Is there a suitable precompiled graphic mode
			printer = AbstractGraphicsPrinter::getMode(vbe_info->bpp, vbe_info->offset_red, vbe_info->offset_green,
			                                           vbe_info->offset_blue, vbe_info->bits_red, vbe_info->bits_green,
			                                           vbe_info->bits_blue);
			if (printer != nullptr) {
				address = reinterpret_cast<void*>(static_cast<uintptr_t>(vbe_info->address));
				size = vbe_info->height * vbe_info->pitch;
				if (size > buffer_size) {
					DBG_VERBOSE << "The current graphic buffer (" << buffer_size << " bytes) is too small (at least "
					            << size << " bytes required)!"  << endl;
					return false;
				} else {
					printer->init(vbe_info->width, vbe_info->height, vbe_info->pitch);
					printer->buffer(buffer[1 - scanout_buffer]);
					return true;
				}
			}
		} else {
			DBG_VERBOSE << "Unsupported graphic mode" << endl;
		}
	}

	// Actually, \ref Multiboot::Framebuffer has everything we need in a real smart structure,
	// however boot loader like PXE don't support them (yet -- quite new, added in the last Multiboot revision).
	Multiboot::Framebuffer * fb = Multiboot::getFramebufferInfo();
	if (fb != nullptr) {
		if (force || fb->type == Multiboot::Framebuffer::RGB) {
			printer = AbstractGraphicsPrinter::getMode(fb->bpp, fb->offset_red, fb->offset_green, fb->offset_blue,
			                                           fb->bits_red, fb->bits_green, fb->bits_blue);
			if (printer != nullptr) {
				address = reinterpret_cast<void*>(static_cast<uintptr_t>(fb->address));
				size = fb->height * fb->pitch;
				if (size > buffer_size) {
					DBG_VERBOSE << "The current graphic buffer (" << buffer_size << " bytes) is too small (at least "
					            << size << " bytes required)!"  << endl;
					return false;
				} else {
					printer->init(fb->width, fb->height, fb->pitch);
					printer->buffer(buffer[1 - scanout_buffer]);
					return true;
				}
			}
		} else {
			DBG_VERBOSE << "Unsupported graphic mode" << endl;
		}
	}

	// Unable to initialize mode
	return false;
}

bool Graphics::switchBuffers() {
	if (!refresh) {
		printer->buffer(buffer[scanout_buffer]);
		scanout_buffer = 1 - scanout_buffer;
		refresh = true;
		return true;
	} else {
		return false;
	}
}

void Graphics::scanoutFrontbuffer() {
	if (refresh) {
		memcpy(address, buffer[scanout_buffer], size);
		refresh = false;
	}
}
