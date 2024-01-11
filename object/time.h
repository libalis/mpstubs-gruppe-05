/*! \file
 *  \brief \ref DateTime structure with conversation to unix timestamp format
 */

#pragma once

#include "types.h"

/*! \brief Date and Time
 *
 * \note only Dates between 1 January 2000 and 31 December 2099 are supported
 */
struct DateTime {
	uint8_t second;   ///< Second (`0` - `59`)
	uint8_t minute;   ///< Minute (`0` - `59`)
	uint8_t hour;     ///< Hour (`0` - `23`)
	uint8_t weekday;  ///< Weekday (`1` (Sun) - `7` (Sat))
	uint8_t day;      ///< Day of Month (`1` - `31`)
	uint8_t month;    ///< Month (`1` - `12`)
	uint16_t year;    ///< Year (`0` (2000) - `99` (2099))

	static const char * const weekdays[8];  ///< Name of weekdays
	static const char * const months[13];   ///< Name of months

	/*! \brief Default constructor
	 */
	DateTime() {}

	/*! \brief Construct from a timestamp (Unix epoch time)
	 */
	explicit DateTime(uint32_t epoch);

	/*! \brief Convert to Unix epoch time
	 *
	 * \return Seconds since 1 January 1970
	 */
	uint32_t toTimestamp() const;
};

template<typename T>
T& operator<<(T &out, const DateTime &dt) {
	out << DateTime::weekdays[dt.weekday] << ", "
	    <<  dt.year << '-';
	if (dt.month < 10) {
		out << '0';
	}
	out << static_cast<unsigned>(dt.month) << '-';

	if (dt.day < 10) {
		out << '0';
	}
	out << static_cast<unsigned>(dt.day) << ' ';

	if (dt.hour < 10) {
		out << '0';
	}
	out << static_cast<unsigned>(dt.hour) << ':';

	if (dt.minute < 10) {
		out << '0';
	}

	out << static_cast<unsigned>(dt.minute) << ':';
	if (dt.second < 10) {
		out << '0';
	}
	out << static_cast<unsigned>(dt.second);

	return out;
}
