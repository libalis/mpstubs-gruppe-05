#include "time.h"

const char * const DateTime::weekdays[8] =
	{ nullptr, "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

const char * const DateTime::months[13] =
	{ nullptr, "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec" };

// inspired by
// https://codereview.stackexchange.com/questions/38275/convert-between-date-time-and-time-stamp-without-using-standard-library-routines
static const unsigned days[4][12] = {
	{    0,   31,   59,   90,  120,  151,  181,  212,  243,  273,  304,  334},
	{  365,  396,  424,  455,  485,  516,  547,  577,  608,  638,  669,  699},
	{  730,  761,  790,  821,  851,  882,  912,  943,  974, 1004, 1035, 1065},  // Leap Year
	{ 1096, 1127, 1155, 1186, 1216, 1247, 1277, 1308, 1339, 1369, 1400, 1430},
};

static const unsigned days4y = 365 * 4 + 1;

DateTime::DateTime(uint32_t epoch) {
	second = epoch % 60;
	epoch /= 60;
	minute = epoch % 60;
	epoch /= 60;
	hour   = epoch % 24;
	epoch /= 24;

	weekday = ((epoch + 4) % 7) + 1;

	unsigned int y = 1970 + epoch / days4y * 4;
	epoch %= days4y;

	for (year = 3; year > 0 && epoch < days[year][0]; year--) {}
	for (month = 11; month > 0 && epoch < days[year][month]; month--) {}

	day = epoch - days[year][month] + 1;
	month++;
	year += y;
}

uint32_t DateTime::toTimestamp() const {
	unsigned y = year - 1970;
	unsigned m = month - 1;
	unsigned d = (y / 4) * days4y + days[y % 4][m] + day - 1;
	return ((d * 24 + hour) * 60 + minute) * 60 + second;
}
