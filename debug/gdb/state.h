/*! \file
 *  \brief \ref GDB_Stub \ref State of the current core
 *  \ingroup debug
 */
#pragma once

#include "types.h"
#include "debug/gdb/handler.h"

#include "machine/fpu.h"

#include "debug/assert.h"

/*! \brief Structure for the core state after a trap
 *
 * GDB allows both reading and changing of registers.
 * These registers are saved at the entry function of a trap (before the actual
 * \ref GDB_Stub::handle is executed) and then converted into this structure --
 * and usually also directly transferred to the connected GDB host.
 * Changes done by the GDB host are first saved in this structure and then
 * written back into the registers right before continuing the actual operating
 * system code.
 */
class State {
 public:
	// Register order required by GDB
	enum RegisterNumber {
		// 64 bit general purpose register
		REG_RAX,
		REG_RBX,
		REG_RCX,
		REG_RDX,
		REG_RSI,
		REG_RDI,
		REG_RBP,
		REG_RSP,
		REG_8,
		REG_9,
		REG_10,
		REG_11,
		REG_12,
		REG_13,
		REG_14,
		REG_15,
		REG_RIP,

		// 32 bit (for GDB) Flag and Segment Register
		REG_EFLAGS,
		REG_CS,
		REG_SS,
		REG_DS,   // not used
		REG_ES,   // not used
		REG_FS,
		REG_GS,

		// 80 bit FPU Data
		REG_ST0,
		REG_ST1,
		REG_ST2,
		REG_ST3,
		REG_ST4,
		REG_ST5,
		REG_ST6,
		REG_ST7,

		// 32 bit FPU State
		REG_FCTRL,  ///< FPU Control Word (fcw)
		REG_FSTAT,  ///< FPU Status Word (fsw)
		REG_FTAG ,  ///< FPU Tag Word (ftw)
		REG_FISEG,  ///< FPU IP Selector (fcs)
		REG_FIOFF,  ///< FPU IP Offset (fip)
		REG_FOSEG,  ///< FPU Operand Pointer Selector (fos)
		REG_FOOFF,  ///< FPU Operand Pointer Offset (foo)
		REG_FOP,    ///< Last Instruction Opcode (fop)

		// 128 bit XMM register
		REG_XMM0,
		REG_XMM1,
		REG_XMM2,
		REG_XMM3,
		REG_XMM4,
		REG_XMM5,
		REG_XMM6,
		REG_XMM7,
		REG_XMM8,
		REG_XMM9,
		REG_XMM10,
		REG_XMM11,
		REG_XMM12,
		REG_XMM13,
		REG_XMM14,
		REG_XMM15,

		// 32 bit XMM status register
		REG_MXCSR,

		REGISTERS  ///< Total number of registers
	};

 private:
	enum Offset {
		OFFSET_GENERAL,
		OFFSET_SEGMENT = REG_EFLAGS,
		OFFSET_FPU_DATA = REG_ST0,
		OFFSET_FPU_STATUS = REG_FCTRL,
		OFFSET_XMM_DATA = REG_XMM0,
		OFFSET_XMM_STATUS = REG_MXCSR,
	};

	enum Size {
		SIZE_GENERAL = OFFSET_SEGMENT - OFFSET_GENERAL,
		SIZE_SEGMENT = OFFSET_FPU_DATA - OFFSET_SEGMENT,
		SIZE_FPU_DATA = OFFSET_FPU_STATUS - OFFSET_FPU_DATA,
		SIZE_FPU_STATUS = OFFSET_XMM_DATA - OFFSET_FPU_STATUS,
		SIZE_XMM_DATA = OFFSET_XMM_STATUS - OFFSET_XMM_DATA,
		SIZE_XMM_STATUS = REGISTERS - OFFSET_XMM_STATUS
	};

	struct Registers {
		uintptr_t general[SIZE_GENERAL];
		uint32_t segment[SIZE_SEGMENT];
		struct FPU::State::ST fpu_data[SIZE_FPU_DATA];
		uint32_t fpu_status[SIZE_FPU_STATUS];
		struct FPU::State::XMM xmm_data[SIZE_XMM_DATA];
		uint32_t xmm_status[SIZE_XMM_STATUS];
	} __attribute__((packed));

	static_assert(REGISTERS == 57, "Wrong x64 GDB Register Size");
	assert_size(Registers, 536);

	Registers registers;

 public:
	/*! \brief Structure to access a register
	 */
	struct Register {
		void* addr;
		size_t size;
		Register() : addr(nullptr), size(0) {}
		template <typename T>
		explicit Register(const T &value) : addr(const_cast<void*>(static_cast<const void*>(&value))), size(sizeof(T)) {}
	};

	/*! \brief Access a previously saved register
	 *
	 * \param reg Register identifier. If \ref State::REGISTERS is used, all
	 *            registers will be returned
	 * \param core 1-indexed Core ID (≤ 0 for current core)
	 * \return Pointer to a structure with address and size of the stored contents
	 */
	static Register get(enum RegisterNumber reg = REGISTERS, int8_t core = 0);

	/// \copydoc get(enum RegisterNumber, int8_t)
	static Register get(uintptr_t reg, int8_t core = 0);

	/*! \brief Read register content from \ref DebugContext and \ref FPU
	 *
	 * \param context Pointer to context of \ref gdb_interrupt_handler
	 */
	static void save(const DebugContext * context);

	/*! \brief Write register contents back \ref DebugContext and \ref FPU
	 *
	 * \param context Pointer to context of \ref gdb_interrupt_handler, which
	 *                will finally be restored after returning from the handler.
	 */
	static void restore(DebugContext * context);
};
