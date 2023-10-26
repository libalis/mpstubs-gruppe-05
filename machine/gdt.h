/*! \file
 *  \brief The \ref GDT "Global Descriptor Table (GDT)".
 */

#pragma once

#include "types.h"

/*! \brief Abstracts the GDT that, primarily, contains descriptors to memory segments.
  *  \ingroup memory
  *
  *  The GDT is a table that primarily contains segment descriptors. Segment descriptors has a size of 8 Bytes and
  *  contains the size, position, access rights, and purpose of such a segment.
  *  Unlike the LDT, the GDT is shared between all processes and may contain TSS and LDT descriptors.
  *  For the kernel, the first entry is required to be a null descriptor and the code and data segments.
  *  To support user-mode processes, additional TSS, code, and data segments for ring 3 must be added.
  *
  *  The base address and size of the GDT are written to the GDTR register during boot (via. `lgdt`).
  *
  * \see [ISDMv3, 2.4.1; Global Descriptor Table Register (GDTR)](intel_manual_vol3.pdf#page=72)
  * \see [ISDMv3, 3.5.1; Segment Descriptor Tables](intel_manual_vol3.pdf#page=99)
 */
namespace GDT {

enum Segments {
	SEGMENT_NULL = 0,
	SEGMENT_KERNEL_CODE,
	SEGMENT_KERNEL_DATA,
};

/*! \brief Unit of the segment limit
 */
enum Granularity {
	GRANULARITY_BYTES   = 0,  ///< Segment limit in Bytes
	GRANULARITY_4KBLOCK = 1   ///< Segment limit in blocks of 4 Kilobytes
};

/*! \brief Descriptor type */
enum DescriptorType {
	DESCRIPTOR_SYSTEM   = 0,  ///< entry is a system segment
	DESCRIPTOR_CODEDATA = 1,  ///< entry is a code/data segment
};

/*! \brief Address width
 */
enum Size {
	SIZE_16BIT = 0,  ///< 16-bit (D/B = 0, L = 0)
	SIZE_32BIT = 2,  ///< 32-bit (D/B = 1, L = 0)
	SIZE_64BIT_CODE = 1,  ///< 64-bit (D/B = 0, L = 1)
	SIZE_64BIT_DATA = 0,  ///< 64-bit (D/B = 0, L = 0)
};

/*! \brief Describes the structure of segment descriptors
 *
 *  A data structure that contains size, position, access rights, and purpose of any segment.
 *  Segment descriptors are used in both the GDT, as well as in LDTs.
 *
 * \see [ISDMv3, 3.4.5; Segment Descriptors](intel_manual_vol3.pdf#page=95)
 * \see [AAPMv2, 4.7 Legacy Segment Descriptors](amd64_manual_vol2.pdf#page=132)
 * \see [AAPMv2, 4.8 Long-Mode Segment Descriptors](amd64_manual_vol2.pdf#page=140)
 */
union SegmentDescriptor {
	// Universally valid values (shared across all segment types)
	struct {
		uint64_t limit_low             : 16;  ///< Least-significant bits of segment size (influenced by granularity!)
		uint64_t base_low              : 24;  ///< Least-significant bits of base address
		uint64_t type                  :  4;  ///< Meaning of those 4 bits depends on descriptor_type below
		DescriptorType descriptor_type :  1;  ///< Descriptor type (influences the meaning of the 3 bits above)
		uint64_t privilege_level       :  2;  ///< Ring for this segment
		bool present                   :  1;  ///< Entry is valid iff set to `true`
		uint64_t limit_high            :  4;  ///< Most-significant bits of segment size
		bool available                 :  1;  ///< Bit which can be used for other purposes (in software)
		uint64_t custom                :  2;  ///< Meaning of those 2 bits relate to descriptor_type and type
		Granularity granularity        :  1;  ///< Unit used as granularity for the segment limit
		uint64_t base_high             :  8;  ///< most-significant bits of base address
	} __attribute__((packed));

	/*! \brief Fields specific for Code Segment
	 * (for debugging purposes)
	 * \see [ISDMv3, 3.4.5.1; Code- and Data-Segment Descriptor Types](intel_manual_vol3.pdf#page=98)
	 */
	struct {
		uint64_t            : 40;  ///< Ignored (set via `limit_low` and `base_low` )

		/* `type` field bits */
		bool code_accessed  :  1;  ///< If set, the code segment was used since the last reset of this value
		bool readable       :  1;  ///< If set, the code is readable (otherwise only executable)

		/*! \brief If set, the execution of code from this segment is only allowed when running at a privilege of
		 *         numerically less than or equal to privilege_level (i.e. the executor has the same or higher
		 *         privileges). However, the executor's privileges remain unchanged.
		 *         For nonconforming code segments (i.e., conforming is set to `0`), execution is allowed only if
		 *         the privileges are equal.
		 *         Execution will cause a GPF in case of privilege violation.
		 */
		bool conforming     :  1;
		bool code           :  1;  ///< Has to be set to `true`

		uint64_t            :  9;  ///< Ignored (set via `privilege_level` ... `available`)

		Size operation_size :  2;  ///< Default address width (`custom` field bit)

		uint64_t            :  0;  ///< Remainder ignored (set via `base_high`)
	} __attribute__((packed));

	/*! \brief Fields specific for Data Segment
	 * (for debugging purposes)
	 * \see [ISDMv3, 3.4.5.1; Code- and Data-Segment Descriptor Types](intel_manual_vol3.pdf#page=98)
	 */
	struct {
		uint64_t            : 40;  ///< Ignored (set via `limit_low` and `base_low`)
		bool data_accessed  :  1;  ///< If set, the data segment was used since the last reset of this value
		bool writeable      :  1;  ///< If set, data is writable (otherwise read only)
		bool expand_down    :  1;  ///< Growing direction for dynamically growing segments
		bool notData        :  1;  ///< Has to be cleared (`false`)
		uint64_t            :  9;  ///< Ignored (set via `privilege_level` ... `available`)
		uint64_t reserved   :  1;  ///< Reserved, always set to `0`!

		/*! \brief Size of the stack pointer (`false` = 16 bit, `true` = 32 bit)
		 *  \warning Has a different meaning in case expand_down is set to `1`.
		 */
		bool big : 1;

		uint64_t : 0;  ///< Remainder ignored
	} __attribute__((packed));

	uint64_t value;  ///!< Merged value; useful for debugging

	/*! \brief Constructor for a specific value */
	constexpr SegmentDescriptor(uint64_t val = 0) : value(val) {}  //NOLINT due to copy-initialization

	/*! \brief Constructor for a code/data GDT entry.
	 *  \param base  Base Address of segment
	 *  \param limit Size of segment
	 *  \param code  Code or data segment
	 *  \param ring  Privilege level
	 *  \param size  Address width
	 */
	constexpr SegmentDescriptor(uintptr_t base, uint32_t limit, bool code, int ring, Size size) :
	    limit_low(limit >> (limit > 0xFFFFF ? 12 : 0) & 0xFFFF),
	    base_low(base & 0xFFFFFF),
	    type(code ? 0xA : 0x2),  // code readable / non-conforming, data writeable and not expanding down
	    descriptor_type(DESCRIPTOR_CODEDATA),
	    privilege_level(ring),
	    present(true),
	    limit_high((limit > 0xFFFFF ? (limit >> 28) : (limit >> 16)) & 0xF),
	    available(false),
	    custom(size),
	    granularity(limit > 0xFFFFF ? GRANULARITY_4KBLOCK : GRANULARITY_BYTES),
	    base_high((base >> 24) & 0xFF) {}

} __attribute__((packed));

static_assert(sizeof(SegmentDescriptor) == 8, "GDT::SegmentDescriptor has wrong size");

/*! \brief Structure that describes a GDT Pointer (aka GDT Descriptor)
 *
 *  It contains both the length (in bytes) of the GDT (minus 1 byte) and the pointer to the GDT.
 *  The pointer to the GDT can be loaded using the instruction `lgdt`.
 *
 *  \note As Intel uses little endian for representing multi-byte values, the GDT::Pointer structure can be used for
 *        16, 32, and 64 bit descriptor tables:
 *        \verbatim
 *         | 16 bit | 16 bit  | 16 bit  | 16 bit  | 16 bit  |
 *         +--------+---------------------------------------+
 * Pointer | limit  | base (up to 64 bit)                   |
 *         +--------+---------+---------+---------+---------+
 *         | used for 16 bit  | ignored...                  |
 *         |      used for 32 bit       | ignored...        |
 *         |                used for 64 bit                 |
 *        \endverbatim
 *
 * \see [ISDMv3, Figure 2-6; Memory Management Registers](intel_manual_vol3.pdf#page=72)
 */
struct Pointer {
	uint16_t limit;  //!< GDT size in bytes (minus 1 byte)
	void * base;     //!< GDT base address

	/*! \brief Constructor (automatic length)
	 * \param desc Array of GDT segment descriptors -- must be defined in the same module!
	 */
	template<typename T, size_t LEN>
	explicit constexpr Pointer(const T (&desc)[LEN]) : limit(LEN * sizeof(T) - 1), base(const_cast<T*>(desc)) {}

	/*! \brief Constructor
	 * \param desc Address of the GDT segment descriptors
	 * \param len  Number of entries
	 */
	constexpr Pointer(void * desc, size_t len) : limit(len * sizeof(SegmentDescriptor) - 1), base(desc) {}

	/*! \brief Set an address
	 *  \note On change, `lgdt` must be executed again
	 *  \param desc Address of the GDT segment descriptors
	 *  \param len  Number of entries
	 */
	void set(void * desc, size_t len) {
		limit = len * sizeof(SegmentDescriptor) - 1;
		base = desc;
	}
} __attribute__((packed));
static_assert(sizeof(Pointer) == 10, "GDT::Pointer has wrong size");

}  // namespace GDT
