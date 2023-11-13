/*
 * Copyright (C) 2016  Matt Borgerson
 * Copyright (C) 2018  Bernhard Heinloth
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "debug/gdb/stub.h"
#include "debug/gdb/state.h"

#include "machine/apic.h"
#include "machine/core.h"
#include "machine/system.h"

#include "debug/assert.h"
#include "debug/output.h"

// ASCII characters for hexadecimal representation
static const char digits[] = "0123456789abcdef";

/*! \brief Map operations to core
 */
static int8_t cpu_ops[127];

/*!\brief Get the corresponding ASCII hex digit character for a value.
 *
 * \param val value
 * \return ASCII hex digit or `-1` if invalid
 */
static char getDigit(int val) {
	return (val >= 0) && (val <= 0xf) ? digits[val] : static_cast<char>(-1);
}

/*!\brief Comparison of two strings
 *
 * \param a first string
 * \param b second string
 * \param len maximum length to compare
 * \return `true` if identical (up to the maximum length)
 */
static bool stringCompare(const char * a, const char * b, size_t len) {
	while (*(a++) == *(b++)) {
		if (--len == 0 || *a == '\0' || *b == '\0') {
			return true;
		}
	}
	return false;
}

/*!\brief Copy string (including `\0`)
 *
 * \param target Pointer to target buffer
 * \param target_len Maximum length of target buffer
 * \param source Pointer to source buffer
 * \param source_len Maximum length of source buffer (or `-1` if it should be determined by the `\0` byte)
 * \return Number of copied characters (bytes)
 */
static size_t stringCopy(char * target, size_t target_len, const char * source, size_t source_len = -1) {
	size_t len = 0;
	while(*source != '\0' && len < target_len && len < source_len) {
		*(target++) = *(source++);
		len++;
	}
	if (len < target_len) {
		*target = '\0';
	}
	return len;
}

/*!\brief Concatenate a source string to the target buffer
 *
 * \param target Pointer to target buffer
 * \param target_len Maximum length of target buffer
 * \param source Pointer to source buffer
 * \param source_len Maximum length of source buffer (or `-1` if it should be determined by the `\0` byte)
 * \return New length of target buffer
 */
static size_t stringConcat(char * target, size_t target_len, const char * source, size_t source_len = -1) {
	size_t len = 0;
	while (*target != '\0' && len < target_len) {
		target++;
		len++;
	}
	return len + stringCopy(target, target_len - len, source, source_len);
}

/*!\brief Concatenate a character to the target buffer
 *
 * \overload stringConcat(char*, size_t, const char*, size_t)
 *
 * \param target Pointer to target buffer
 * \param target_len Maximum length of target buffer
 * \param source Source character
 * \return New length of target buffer
 */
static size_t stringConcat(char * target, size_t target_len, char source) {
	return stringConcat(target, target_len, &source, 1);
}

/*!\brief Get the corresponding value for a ASCII digit character.
 *
 * \param digit ASCII digit character
 * \param base Base to convert (supports bases 2 to 16)
 * \return value or `-1` if invalid
 */
static int getValue(char digit, int base) {
	int value;

	if ((digit >= '0') && (digit <= '9')) {
		value = digit-'0';
	} else if ((digit >= 'a') && (digit <= 'f')) {
		value = digit-'a'+0xa;
	} else if ((digit >= 'A') && (digit <= 'F')) {
		value = digit-'A'+0xa;
	} else {
		return -1;
	}

	return (value < base) ? value : -1;
}

/*!\brief Determine if this is a printable ASCII character
 *
 * \param ch character to check
 * \retval true printable
 * \retval false not printable
 */
static bool isPrintableChar(char ch) {
	return (ch >= 0x20 && ch <= 0x7e);
}

/*!\brief Get integer value for a string representation.
 *
 * \param str String to convert. It it starts with `+` or `-` it will be
 *            signed accordingly.
 * \param len length of string
 * \param base Base for the conversion.
 *             In case the value is set to `0`, the base will be determined:
 *             if string starts with `0x` or `0X` it is hexadecimal (base 16),
 *             otherwise decimal representation (base 10).
 * \param endptr If specified, it will point to the last non-digit in the
 *               string. If there are no digits in the string, it will be set
 *               to `nullptr`.
 * \return recognized integer
 */
static intptr_t stringToInteger(const char *str, size_t len, int base, const char **endptr) {
	intptr_t value = 0;
	size_t pos = 0;
	intptr_t sign = 1;
	int valid = 0;

	if (endptr != nullptr) {
		*endptr = nullptr;
	}

	if (len < 1) {
		return 0;
	}

	// Detect negative numbers
	if (str[pos] == '-') {
		sign = -1;
		pos += 1;
	} else if (str[pos] == '+') {
		sign = 1;
		pos += 1;
	}

	// Detect '0x' hex prefix
	if ((pos + 2 < len) && (str[pos] == '0') &&
		((str[pos+1] == 'x') || (str[pos+1] == 'X'))) {
		base = 16;
		pos += 2;
	}

	if (base == 0) {
		base = 10;
	}

	for (; (pos < len) && (str[pos] != '\x00'); pos++) {
		int tmp = getValue(str[pos], base);
		if (tmp == -1) {
			break;
		}

		value = value*base + tmp;
		valid = 1;  // at least one digit is valid
	}

	if (valid == 0) {
		return 0;
	}

	if (endptr != nullptr) {
		*endptr = str+pos;
	}

	value *= sign;

	return value;
}

/*!\brief Encode data to its hex-value representation in a buffer.
 *
 * \param buf Pointer to target buffer for encoded data
 * \param buf_len Size of target buffer
 * \param data Source data buffer (raw representation)
 * \param data_len Size if source data
 * \return number of bytes written to target buffer or `-1` if the buffer is too small
 */
static int encodeHex(char *buf, size_t buf_len, const char *data, size_t data_len) {
	// buffer too small
	if (buf_len < data_len * 2) {
		return -1;
	}

	for (size_t pos = 0; pos < data_len; pos++) {
		*buf++ = getDigit((data[pos] >> 4) & 0xf);
		*buf++ = getDigit((data[pos]) & 0xf);
	}

	return static_cast<int>(data_len * 2);
}

/*!\brief Encode data to its hex-value representation in a buffer.
 *
 * \overload encodeHex(char*, size_t, const char*, size_t)
 *
 * \param buf Pointer to target buffer for encoded data
 * \param buf_len Size of target buffer
 * \param reg Source register
 * \return number of bytes written to target buffer or `-1` if the buffer is too small
 */
static int encodeHex(char *buf, size_t buf_len, const State::Register reg) {
	return encodeHex(buf, buf_len, static_cast<const char*>(reg.addr), reg.size);
}

/*!\brief Decode data from its hex-value representation to a buffer.
 *
 * \param buf Source buffer with encoded data
 * \param buf_len Size of source buffer
 * \param data Target data buffer (raw representation)
 * \param data_len Size if target data
 * \retval 0 on success
 * \retval -1` if the buffer is too small
 */
static int decodeHex(const char *buf, size_t buf_len, char *data, size_t data_len) {
	// Buffer too small
	if (buf_len != data_len * 2) {
		return -1;
	}

	for (size_t pos = 0; pos < data_len; pos++) {
		// Decode high nibble
		int tmp = getValue(*buf++, 16);
		if (tmp == -1) {
			// Buffer contained junk
			assert(false);
			return -1;
		}

		data[pos] = static_cast<char>((tmp << 4));

		// Decode low nibble
		tmp = getValue(*buf++, 16);
		if (tmp == -1) {
			// Buffer contained junk
			assert(false);
			return -1;
		}
		data[pos] |= static_cast<char>(tmp);
	}

	return 0;
}

/*!\brief Decode data from its hex-value representation to a buffer.
 *
 * \overload decodeHex(const char*, size_t, char*, size_t)
 *
 * \param buf Source buffer with encoded data
 * \param buf_len Size of source buffer
 * \param reg Target register
 * \retval 0 on success
 * \retval -1` if the buffer is too small
 */
static int decodeHex(const char *buf, size_t buf_len, State::Register reg) {
	return decodeHex(buf, buf_len, static_cast<char*>(reg.addr), reg.size);
}

/*!\brief Encode data to its binary representation in a buffer
 *
* \param buf Pointer to target buffer for encoded data
* \param buf_len Size of target buffer
* \param data Source data buffer (raw representation)
* \param data_len Size if source data
* \return number of bytes written to target buffer or `-1` if the buffer is too small
 */
static int encodeBinary(char *buf, size_t buf_len, const char *data, size_t data_len) {
	size_t buf_pos = 0;
	for (size_t data_pos = 0; data_pos < data_len; data_pos++) {
		if (data[data_pos] == '$' || data[data_pos] == '#' || data[data_pos] == '}' || data[data_pos] == '*') {
			if (buf_pos+1 >= buf_len) {
				assert(0);
				return -1;
			}
			buf[buf_pos++] = '}';
			buf[buf_pos++] = data[data_pos] ^ 0x20;
		} else {
			if (buf_pos >= buf_len) {
				assert(0);
				return -1;
			}
			buf[buf_pos++] = data[data_pos];
		}
	}

	return buf_pos;
}

/*!\brief Decode data from its bin-value representation to a buffer.
 *
 * \param buf Source buffer with encoded data
 * \param buf_len Size of source buffer
 * \param data Target data buffer (raw representation)
 * \param data_len Size if target data
 * \return number of bytes decoded or `-1` if the buffer is too small
 */
static int decodeBinary(const char *buf, size_t buf_len, char *data, size_t data_len) {
	size_t data_pos = 0;
	for (size_t buf_pos = 0; buf_pos < buf_len; buf_pos++) {
		if (data_pos >= data_len) {
			// Output buffer overflow
			assert(false);
			return -1;
		}
		if (buf[buf_pos] == '}') {
			// The next byte is escaped!
			if (buf_pos+1 >= buf_len) {
				// There's an escape character, but no escaped character following the escape character
				assert(false);
				return -1;
			}
			buf_pos += 1;
			data[data_pos++] = buf[buf_pos] ^ 0x20;
		} else {
			data[data_pos++] = buf[buf_pos];
		}
	}

	return data_pos;
}

int GDB_Stub::receiveAck(void) {
	// Wait for packet ack
	int response = read();
	switch (response) {
		case '+':
			return 0;  // Packet acknowledged

		case '-':
			return 1;  // Packet negative acknowledged

		default:
			// Bad response!
			DBG_VERBOSE << "GDB: received bad packet response: " << hex << response << endl;
			return -1;
	}
}

int GDB_Stub::checksum(const char *buf, size_t len) {
	unsigned char csum = 0;
	while (len-- != 0) {
		csum += *buf++;
	}
	return csum;
}

int GDB_Stub::sendPacket(const char *pkt_data, size_t pkt_len) {
	// Send packet start
	if (write('$') == -1) {
		return -1;
	}

	// Debug output to debug the debugger
	if (debug) {
		DBG_VERBOSE << "GDB: ->";
		for (size_t p = 0; p < pkt_len; p++) {
			if (isPrintableChar(pkt_data[p])) {
				DBG_VERBOSE << static_cast<char>(pkt_data[p]);
			} else {
				DBG_VERBOSE << hex << (pkt_data[p] & 0xff);
			}
		}
		DBG_VERBOSE << endl;
	}

	// Send packet data
	if (writeString(pkt_data, pkt_len) == -1) {
		return -1;
	}

	// Send the checksum
	char buf[3];
	buf[0] = '#';
	char csum = checksum(pkt_data, pkt_len);
	if ((encodeHex(buf+1, sizeof(buf)-1, &csum, 1) == -1) || (writeString(buf, sizeof(buf)) == -1)) {
		return -1;
	}

	// Wait for acknowledgment
	return receiveAck();
}

int GDB_Stub::receivePacket(char *pkt_buf, size_t pkt_buf_len, size_t *pkt_len) {
	// Detected start of packet
	while (read() != '$') {}

	// Read until checksum
	*pkt_len = 0;
	while (true) {
		int data;
		if ((data = read()) == -1) {  // Error receiving character
			return -1;
		} else if (data == '#') {  // End of packet
			break;
		} else {
			if (*pkt_len >= pkt_buf_len) {  // Check for space
				DBG_VERBOSE << "GDB: packet buffer overflow" << endl;
				return -1;
			}

			// Store character and update checksum
			pkt_buf[(*pkt_len)++] = static_cast<char>(data);
		}
	}

	// Debug output to debug the debugger
	if (debug) {
		DBG_VERBOSE << "GDB: <- ";
		for (size_t p = 0; p < *pkt_len; p++) {
			if (isPrintableChar(pkt_buf[p])) {
				DBG_VERBOSE << static_cast<char>(pkt_buf[p]);
			} else {
				DBG_VERBOSE << hex << (pkt_buf[p] & 0xff);
			}
		}
		DBG_VERBOSE << endl;;
	}

	// Receive the checksum
	char buf[2];
	char actual_csum;
	char expected_csum;
	if ((readString(buf, sizeof(buf), 2) == -1) || (decodeHex(buf, 2, &expected_csum, 1) == -1)) {
		return -1;
	} else if ((actual_csum = checksum(pkt_buf, *pkt_len)) != expected_csum) {  // Verify checksum
		// Send packet nack
		DBG_VERBOSE << "GDB: received packet with bad checksum (" << static_cast<int>(actual_csum)
		            << " instead of " << static_cast<int>(expected_csum) << ")" << endl;
		write('-');
		return -1;
	} else {
		// Send packet ack
		write('+');
		return 0;
	}
}

int GDB_Stub::memRead(char *buf, size_t buf_len, uintptr_t addr, size_t len, bool hex) {
	static char data[512];
	if (len > sizeof(data)) {
		return -1;
	}

	// Read from system memory
	for (size_t pos = 0; pos < len; pos++) {
		data[pos] = *reinterpret_cast<volatile char *>(static_cast<uintptr_t>(addr+pos));
	}

	// Encode data
	return hex ? encodeHex(buf, buf_len, data, len) : encodeBinary(buf, buf_len, data, len);
}

int GDB_Stub::memWrite(const char *buf, size_t buf_len, uintptr_t addr, size_t len, bool hex) {
	static char data[512];

	if (len > sizeof(data)) {
		return -1;
	}

	// Decode data
	if ((hex ? decodeHex(buf, buf_len, data, len) : decodeBinary(buf, buf_len, data, len)) == -1) {
		return -1;
	}

	// Write to system memory
	for (size_t pos = 0; pos < len; pos++) {
		*reinterpret_cast<volatile char *>(static_cast<uintptr_t>(addr+pos)) = data[pos];
	}

	return 0;
}

void GDB_Stub::sysContinue(void) {
	uint32_t *eflags = static_cast<uint32_t*>(State::get(State::REG_EFLAGS).addr);
	if (eflags != nullptr) {
		*eflags &= ~(1 << 8);
	}
}

void GDB_Stub::sysStep(void) {
	uint32_t *eflags = static_cast<uint32_t*>(State::get(State::REG_EFLAGS).addr);
	if (eflags != nullptr) {
		*eflags |= 1 << 8;
	}
}

int GDB_Stub::sendOkPacket() {
	return sendPacket("OK", 2);
}

int GDB_Stub::sendSignalPacket(char *buf, size_t buf_len, char signal) {
	if (buf_len < 4) {
		return -1;
	}

	buf[0] = 'S';
	int size = encodeHex(&buf[1], buf_len-1, &signal, 1);
	return size == -1 ? -1 : sendPacket(buf, 1 + size);
}

int GDB_Stub::sendErrorPacket(char *buf, size_t buf_len, char error) {
	if (buf_len < 4) {
		return -1;
	}

	buf[0] = 'E';
	int size = encodeHex(&buf[1], buf_len-1, &error, 1);
	return size == -1 ? -1 : sendPacket(buf, 1 + size);
}

int GDB_Stub::writeString(const char *buf, size_t len) {
	while (len-- != 0) {
		if (write(*buf++) == -1) {
			return -1;
		}
	}
	return 0;
}

int GDB_Stub::readString(char *buf, size_t buf_len, size_t len) {
	// Buffer to small
	if (buf_len < len) {
		return -1;
	}

	while (len-- != 0) {
		char c = read();
		if (c == -1) {
			return -1;
		}
		*buf++ = c;
	}

	return 0;
}

void GDB_Stub::handle(void) {
	uintptr_t addr = 0;
	static char pkt_buf[2048];
	int status;
	size_t length;
	size_t pkt_len;
	const char *ptr_next;

	sendSignalPacket(pkt_buf, sizeof(pkt_buf), 0);

	while (receivePacket(pkt_buf, sizeof(pkt_buf), &pkt_len) != -1) {
		if (pkt_len == 0) {  // Received empty packet..
			continue;
		}

		ptr_next = pkt_buf;
		unsigned char buf0 = static_cast<unsigned char>(pkt_buf[0]);
		int8_t cpu = cpu_ops[buf0 < sizeof(cpu_ops) ?buf0 : 0];
		// Handle one letter commands
		switch (pkt_buf[0]) {
			// Calculate remaining space in packet from ptr_next position
			#define token_remaining_buf (pkt_len-(ptr_next-pkt_buf))

			// Expecting a separator. If not present, go to error
			#define token_expect_seperator(c) \
				do { \
					if (!ptr_next || *ptr_next != (c)) { \
						goto error; \
					} else { \
						ptr_next += 1; \
					} \
				} while (0)

			// Expecting an integer argument. If not present, go to error
			#define token_expect_integer_arg(arg) \
				do { \
					(arg) = stringToInteger(ptr_next, token_remaining_buf, 16, &ptr_next); \
					if (!ptr_next) { \
						goto error; \
					} \
				} while (0)

			// [H OP Core] Set core for operation
			case 'H':
				if (static_cast<unsigned char>(pkt_buf[1]) >= sizeof(cpu_ops)) {
					goto error;
				}
				ptr_next += 2;
				token_expect_integer_arg(addr);
				cpu_ops[static_cast<unsigned char>(pkt_buf[1])] = static_cast<int8_t>(addr);

				sendOkPacket();
				break;

			// [qSTRING...] query command
			case 'q':
				pkt_buf[pkt_len]='\0';

				if (stringCompare(pkt_buf, "qC", pkt_len)) {  // [qC] get current core ID
					stringCopy(pkt_buf, sizeof(pkt_buf), "QC");
					stringConcat(pkt_buf, sizeof(pkt_buf), static_cast<char>('1' + Core::getID()));
					sendPacket(pkt_buf, 3);
				} else if (stringCompare(pkt_buf, "qfThreadInfo", pkt_len)) {  // [qfThreadInfo] get core IDs
					length = stringCopy(pkt_buf, sizeof(pkt_buf), "m1");
					for (unsigned i = 1; i < Core::countOnline(); ++i) {
						stringConcat(pkt_buf, sizeof(pkt_buf), ',');
						length = stringConcat(pkt_buf, sizeof(pkt_buf), i + '1');
					}
					sendPacket(pkt_buf, length);
				} else if (stringCompare(pkt_buf, "qsThreadInfo", pkt_len)) {  // [qsThreadInfo] get additional core IDs
					stringCopy(pkt_buf, sizeof(pkt_buf), "l");
					sendPacket(pkt_buf, 1);
				} else if (stringCompare(pkt_buf, "qThreadExtraInfo,", pkt_len)) {  // [qThreadExtraInfo,Core] get info about core
					ptr_next += 17;
					token_expect_integer_arg(addr);
					if (addr > Core::countOnline()) {
						goto error;
					}

					stringCopy(pkt_buf, sizeof(pkt_buf), "43657265233");           // "Core "
					stringConcat(pkt_buf, sizeof(pkt_buf), (addr - 1) + '0');
					stringConcat(pkt_buf, sizeof(pkt_buf), "202f204c4150494320233");  // " / LAPIC: "
					length = stringConcat(pkt_buf, sizeof(pkt_buf), APIC::getLAPICID(addr - 1) + '0');
					sendPacket(pkt_buf, length);
				} else {  // Unsupported function
					sendPacket(nullptr, 0);
				}
				break;

			// [g] Read all registers
			case 'g':
				// Encode registers
				status = encodeHex(pkt_buf, sizeof(pkt_buf), State::get(State::REGISTERS, cpu));

				if (status == -1) {
					goto error;
				}

				pkt_len = status;
				sendPacket(pkt_buf, pkt_len);
				break;

			// [G XX...] write all register
			case 'G':
				status = decodeHex(pkt_buf+1, pkt_len-1, State::get(State::REGISTERS, cpu));

				if (status == -1) {
					goto error;
				}

				sendOkPacket();
				break;

			// [p n] Read a register
			case 'p':
				ptr_next += 1;
				token_expect_integer_arg(addr);

				if (addr >= State::REGISTERS) {
					goto error;
				} else {
					status = encodeHex(pkt_buf, sizeof(pkt_buf), State::get(addr, cpu));
				}
				if (status == -1) {
					goto error;
				}
				sendPacket(pkt_buf, status);
				break;

			// [P n...=r...] Write a register
			case 'P':
				ptr_next += 1;
				token_expect_integer_arg(addr);
				token_expect_seperator('=');

				if (addr >= State::REGISTERS) {
					goto error;
				} else {
					status = decodeHex(ptr_next, token_remaining_buf, State::get(addr, cpu));
				}
				if (status == -1) {
					goto error;
				}

				sendOkPacket();
				break;
			// [T Core] Core online?
			case 'T':
				ptr_next += 1;
				token_expect_integer_arg(addr);
				if (addr > Core::countOnline()) {
					goto error;
				}

				sendOkPacket();
				break;

			// [k] Restart system
			case 'k':
				System::reboot();
				break;

			// [m addr,length] Read system memory
			case 'm':
				ptr_next += 1;
				token_expect_integer_arg(addr);
				token_expect_seperator(',');
				token_expect_integer_arg(length);

				// Read Memory
				status = memRead(pkt_buf, sizeof(pkt_buf), addr, length, true);
				if (status == -1) {
					goto error;
				}

				sendPacket(pkt_buf, status);
				break;

			// [M addr,length:XX..] Write system memory
			case 'M':
				ptr_next += 1;
				token_expect_integer_arg(addr);
				token_expect_seperator(',');
				token_expect_integer_arg(length);
				token_expect_seperator(':');

				// Write memory
				status = memWrite(ptr_next, token_remaining_buf, addr, length, true);
				if (status == -1) {
					goto error;
				}

				sendOkPacket();
				break;

			// [X addr,length:XX..] Write system memory (Binary)
			case 'X':
				ptr_next += 1;
				token_expect_integer_arg(addr);
				token_expect_seperator(',');
				token_expect_integer_arg(length);
				token_expect_seperator(':');

				// Write memory
				status = memWrite(ptr_next, token_remaining_buf, addr, length, false);
				if (status == -1) {
					goto error;
				}

				sendOkPacket();
				break;

			// [c [addr]] Continue
			case 'c':
				sysContinue();
				return;

			// [s [addr]] Single-step
			case 's':
				sysStep();
				return;

			// [?] Query signal (trap)
			case '?':
				sendSignalPacket(pkt_buf, sizeof(pkt_buf), 0);
				break;

			// Unsupported Command
			default:
				if (debug) {
					DBG_VERBOSE << "GDB: Unsupported Command '" << pkt_buf[0] << "'" << endl;
				}
				sendPacket(nullptr, 0);
		}

		continue;

	// Error handling
	error:
		sendErrorPacket(pkt_buf, sizeof(pkt_buf), 0x00);

		#undef token_remaining_buf
		#undef token_expect_seperator
		#undef token_expect_integer_arg
	}
}
