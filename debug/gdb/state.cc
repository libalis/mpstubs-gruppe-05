#include "debug/gdb/state.h"
#include "debug/gdb/handler.h"

#include "machine/core.h"

/*! \brief Storage for the current core register state
 *
 * Updated at the beginning of interrupt handling and written back at the end
 * (with possibly modified values)
  */
static State state[Core::MAX];

/*! \brief Storage for the current \ref FPU state of the core
 *
 * Stored when \ref State::save is called using `FXSAVE` (in \ref FPU::save())
 * and later restored with `FXRSTOR` (in \ref FPU::restore()) -- with modified
 * values if necessary
 */
static FPU::State fpu_state[Core::MAX];

State::Register State::get(enum RegisterNumber reg, int8_t core) {
	return get(static_cast<uintptr_t>(reg), core);
}

State::Register State::get(uintptr_t reg, int8_t core) {
	unsigned coreid = core <= 0 ? Core::getID() : (core - 1);

	assert(coreid < Core::MAX);

	struct Registers &registers = state[coreid].registers;

	if (reg > REGISTERS) {
		return Register();
	} else if (reg == REGISTERS) {
		return Register(registers);
	} else if (reg >= OFFSET_XMM_STATUS) {
		return Register(registers.xmm_status[reg - OFFSET_XMM_STATUS]);
	} else if (reg >= OFFSET_XMM_DATA) {
		return Register(registers.xmm_data[reg - OFFSET_XMM_DATA]);
	} else if (reg >= OFFSET_FPU_STATUS) {
		return Register(registers.fpu_status[reg - OFFSET_FPU_STATUS]);
	} else if (reg >= OFFSET_FPU_DATA) {
		return Register(registers.fpu_data[reg - OFFSET_FPU_DATA]);
	} else if (reg >= OFFSET_SEGMENT) {
		return Register(registers.segment[reg - OFFSET_SEGMENT]);
	} else {  // if (reg >= OFFSET_GENERAL)
		return Register(registers.general[reg - OFFSET_GENERAL]);
	}
}

void State::save(const DebugContext * const context) {
	unsigned coreid = Core::getID();
	assert(coreid < Core::MAX);

	struct Registers &registers = state[coreid].registers;

	FPU::State &fpu = fpu_state[coreid];

	// Copy registers from interrupt context
	registers.general[REG_RAX - OFFSET_GENERAL] = context->rax;
	registers.general[REG_RBX - OFFSET_GENERAL] = context->rbx;
	registers.general[REG_RCX - OFFSET_GENERAL] = context->rcx;
	registers.general[REG_RDX - OFFSET_GENERAL] = context->rdx;
	registers.general[REG_RSI - OFFSET_GENERAL] = context->rsi;
	registers.general[REG_RDI - OFFSET_GENERAL] = context->rdi;
	registers.general[REG_RBP - OFFSET_GENERAL] = context->rbp;
	registers.general[REG_RSP - OFFSET_GENERAL] = context->rsp;
	registers.general[REG_8   - OFFSET_GENERAL] = context->r8;
	registers.general[REG_9   - OFFSET_GENERAL] = context->r9;
	registers.general[REG_10  - OFFSET_GENERAL] = context->r10;
	registers.general[REG_11  - OFFSET_GENERAL] = context->r11;
	registers.general[REG_12  - OFFSET_GENERAL] = context->r12;
	registers.general[REG_13  - OFFSET_GENERAL] = context->r13;
	registers.general[REG_14  - OFFSET_GENERAL] = context->r14;
	registers.general[REG_15  - OFFSET_GENERAL] = context->r15;
	registers.general[REG_RIP - OFFSET_GENERAL] = context->rip;

	registers.segment[REG_CS     - OFFSET_SEGMENT] = static_cast<uint32_t>(context->cs);
	registers.segment[REG_SS     - OFFSET_SEGMENT] = static_cast<uint32_t>(context->ss);
	registers.segment[REG_FS     - OFFSET_SEGMENT] = static_cast<uint32_t>(context->fs);
	registers.segment[REG_GS     - OFFSET_SEGMENT] = static_cast<uint32_t>(context->gs);

	registers.segment[REG_EFLAGS - OFFSET_SEGMENT] = static_cast<uint32_t>(context->eflags);

	// Store FPU state
	fpu.save();

	// FPU context
	for (unsigned i = 0; i < SIZE_FPU_DATA; i++) {
		registers.fpu_data[i] = fpu.st[i].value;
	}

	registers.fpu_status[REG_FCTRL - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.fcw);
	registers.fpu_status[REG_FSTAT - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.fsw);
	registers.fpu_status[REG_FTAG  - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.ftw);
	registers.fpu_status[REG_FISEG - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.ip_seg);
	registers.fpu_status[REG_FIOFF - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.ip_off);
	registers.fpu_status[REG_FOSEG - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.dp_seg);
	registers.fpu_status[REG_FOOFF - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.dp_off);
	registers.fpu_status[REG_FOP   - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.fop);

	for (unsigned i = 0; i < SIZE_XMM_DATA; i++) {
		registers.xmm_data[i] = fpu.xmm[i];
	}

	registers.xmm_status[REG_MXCSR - OFFSET_XMM_STATUS] = fpu.mxcsr;
}

void State::restore(DebugContext * const context) {
	unsigned coreid = Core::getID();
	assert(coreid < Core::MAX);

	struct Registers &registers = state[coreid].registers;

	FPU::State &fpu = fpu_state[coreid];

	context->rax    = registers.general[REG_RAX - OFFSET_GENERAL];
	context->rbx    = registers.general[REG_RBX - OFFSET_GENERAL];
	context->rcx    = registers.general[REG_RCX - OFFSET_GENERAL];
	context->rdx    = registers.general[REG_RDX - OFFSET_GENERAL];
	context->rsi    = registers.general[REG_RSI - OFFSET_GENERAL];
	context->rdi    = registers.general[REG_RDI - OFFSET_GENERAL];
	context->rbp    = registers.general[REG_RBP - OFFSET_GENERAL];
	context->rsp    = registers.general[REG_RSP - OFFSET_GENERAL];
	context->r8     = registers.general[REG_8   - OFFSET_GENERAL];
	context->r9     = registers.general[REG_9   - OFFSET_GENERAL];
	context->r10    = registers.general[REG_10  - OFFSET_GENERAL];
	context->r11    = registers.general[REG_11  - OFFSET_GENERAL];
	context->r12    = registers.general[REG_12  - OFFSET_GENERAL];
	context->r13    = registers.general[REG_13  - OFFSET_GENERAL];
	context->r14    = registers.general[REG_14  - OFFSET_GENERAL];
	context->r15    = registers.general[REG_15  - OFFSET_GENERAL];
	context->rip    = registers.general[REG_RIP - OFFSET_GENERAL];

	context->cs     = registers.segment[REG_CS     - OFFSET_SEGMENT];
	context->ss     = registers.segment[REG_SS     - OFFSET_SEGMENT];
	context->fs     = registers.segment[REG_FS     - OFFSET_SEGMENT];
	context->gs     = registers.segment[REG_GS     - OFFSET_SEGMENT];

	context->eflags = registers.segment[REG_EFLAGS - OFFSET_SEGMENT];

	// FPU registers
	for (unsigned i = 0; i < SIZE_FPU_DATA; i++) {
		fpu.st[i].value = registers.fpu_data[i];
	}

	registers.fpu_status[REG_FCTRL - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.fcw);
	registers.fpu_status[REG_FSTAT - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.fsw);
	registers.fpu_status[REG_FTAG  - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.ftw);
	registers.fpu_status[REG_FISEG - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.ip_seg);
	registers.fpu_status[REG_FIOFF - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.ip_off);
	registers.fpu_status[REG_FOSEG - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.dp_seg);
	registers.fpu_status[REG_FOOFF - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.dp_off);
	registers.fpu_status[REG_FOP   - OFFSET_FPU_STATUS] = static_cast<uint32_t>(fpu.fop);

	for (unsigned i = 0; i < SIZE_XMM_DATA; i++) {
		fpu.xmm[i] = registers.xmm_data[i];
	}

	fpu.mxcsr = registers.xmm_status[REG_MXCSR - OFFSET_XMM_STATUS];
	fpu.restore();
}
