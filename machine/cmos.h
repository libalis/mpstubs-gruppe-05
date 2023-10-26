 /*! \file
 *  \brief Controlling the \ref CMOS "complementary metal oxide semiconductor (CMOS)"
 */

#pragma once

#include "types.h"

/*! \brief CMOS
 * \ingroup CMOS
 */
namespace CMOS {

enum Register {
	REG_SECOND          = 0x0,  ///< RTC
	REG_ALARM_SECOND    = 0x1,  ///< RTC
	REG_MINUTE          = 0x2,  ///< RTC
	REG_ALARM_MINUTE    = 0x3,  ///< RTC
	REG_HOUR            = 0x4,  ///< RTC
	REG_ALARM_HOUR      = 0x5,  ///< RTC
	REG_WEEKDAY         = 0x6,  ///< RTC
	REG_DAYOFMONTH      = 0x7,  ///< RTC
	REG_MONTH           = 0x8,  ///< RTC
	REG_YEAR            = 0x9,  ///< RTC
	REG_STATUS_A        = 0xa,  ///< RTC
	REG_STATUS_B        = 0xb,  ///< RTC
	REG_STATUS_C        = 0xc,  ///< RTC
	REG_STATUS_D        = 0xd,  ///< RTC
	REG_STATUS_DIAGNOSE = 0xe,
	REG_STATUS_SHUTDOWN = 0xf
};

uint8_t read(enum Register reg);
void write(enum Register reg, uint8_t value);

namespace NMI {
void enable();
void disable();
bool isEnabled();
}  // namespace NMI
}  // namespace CMOS
