/*! \file
 *  \brief \ref GDB_Stub contains the necessary functionality for remote debugging
 *  \ingroup debug
 */

/*! \defgroup debug Debugging Tools
 *  \brief Support debugging of the operating system
 */

#pragma once

#include "debug/gdb/handler.h"
#include "machine/serial.h"

/*! \brief Manual debug breakpoint.
 *
 * Triggers a breakpoint trap
 */
inline void breakpoint(void) {
	asm volatile("int $3;\r\n" ::: "memory");
}

/*! \brief Remote stub for GNU Debugger (GDB)
 *
 * Allows remote debugging of the operating system with GDB on real (bare metal)
 * hardware (but also in the emulator) by implementing the main part of the
 * [GDB Remote Serial Protocols (RSP)](https://sourceware.org/gdb/onlinedocs/gdb/Remote-Protocol.html),
 * installing own interrupt handler routines for traps and communicating with
 * the GDB host over the serial port.
 *
 * To use this feature, GDB must be started on the host with the identical kernel
 * binary file as the operating system on the target hardware runs with --
 * ideally in the kernels source code directory, because then the source code
 * can be embedded in the debug output of the GDB host.
 *
 * In addition, the settings for serial transmission must be identical on both hardware and GDB.
 *
 * Example:
 *
 *      ~> ssh i4stubs-serial
 *      cip6d0:~> cd mpstubs
 *      cip6d0:~/mpstubs> make netboot
 *      cip6d0:~/mpstubs> gdb .build/system64
 *      GNU gdb (Debian 7.12-6) 7.12.0.20161007-git
 *      [...]
 *      Reading symbols from /proj/i4stubs/student/uj66ojab/kernel...done.
 *      (gdb) set arch i386:x86-64

 *      (gdb) set serial baud 9600
 *      (gdb) target remote /dev/ttyBS1
 *      Remote debugging using /dev/ttyBS1
 *      main () at main.cc:87
 *
 * \note The last (current) trap/interrupt vector handled by the GDB Stub can be
 *       queried inside the GDB shell using the command `print GDB_Stub::signal`.
 *
 * \note GDB already comes with the [i386-stub.c](https://sourceware.org/git/gitweb.cgi?p=binutils-gdb.git;a=blob;f=gdb/stubs/i386-stub.c;hb=HEAD).
 *       However, it is ugly, poorly maintained and not very easy to integrate
 *       into this object-oriented operating system.
 *       Therefore we use a revised version of [Matt Borgersons gdbstub](https://github.com/mborgerson/gdbstub)
 *       (released 2016 under the GPL v2 license).
 */
class GDB_Stub : public Serial {
	// Prevent copies and assignments
	GDB_Stub(const GDB_Stub&)            = delete;
	GDB_Stub& operator=(const GDB_Stub&) = delete;

	/*! \brief Additional debug outputs on the target system (useful for debugging the debugger stub)
	 */
	const bool debug;

 public:
	/*! \brief Last trap/interrupt vector handled in GDB
	 */
	static int signal;

	/*! \brief constructor
	 *
	 * configures interrupt handler and serial interface (as `8N1`)
	 *
	 * \param wait Wait for a GDB connection after configuration
	 * \param debug_output Debug the stub by printing the communication on the DBG output
	 *                     (could be useful while extending the RSP)
	 * \param port COM port for the serial connection
	 * \param baud_rate Baud Rate, default for GDB is `9600` (can be a bottleneck)
	 */
	explicit GDB_Stub(bool wait = false, bool debug_output = false, ComPort port = COM1, BaudRate baud_rate = BAUD_9600);

 protected:
	/*! \brief Handling traps
	 *
	 * Called by the \ref gdb_interrupt_handler "generic debug interrupt handler"
	 * after the current core state has been stored.
	 *
	 * This contains the heart functionality of the \ref GDB_Stub, including the
	 * communication with the GDB host.
	 */
	void handle();

	/*! \brief Allow the generic GDB interrupt handler to access the protected methods of this class
	 */
	friend void gdb_interrupt_handler(DebugContext * context);

	/*! \brief Send a string via the serial connection
	 *
	 * \param buf Pointer to buffer
	 * \param len Size of buffer (or characters to transmit)
	 * \retval  0 on success
	 * \retval -1 if no or not all bytes have been sent
	 */
	int writeString(const char *buf, size_t len);

	/*! \brief Receive a string via the serial connection
	 *
	 * \param buf Pointer to buffer
	 * \param buf_len Size of buffer
	 * \param len Size of received characters
	 * \retval  0 on success
	 * \retval -1 if no or not all bytes have been received
	 */
	int readString(char *buf, size_t buf_len, size_t len);

	/*! \brief Transmits a packet of data.
	 *
	 * Packets are of the form:
	 * \code{.gdb}
	 *    $<packet-data>#<checksum>
	 * \endcode
	 *
	 * \param pkt_data Pointer to packet buffer
	 * \param pkt_len Size of packet buffer
	 * \retval  0 if the packet was transmitted and acknowledged
	 * \retval  1 if the packet was transmitted but not acknowledged
	 * \retval -1 otherwise
	 */
	int sendPacket(const char *pkt_data, size_t pkt_len);

	/*! \brief Receives a packet of data
	 * assuming a 7-bit clean connection.
	 *
	 * \param pkt_buf Pointer to packet buffer
	 * \param pkt_buf_len Size of packet buffer
	 * \param pkt_len Size of packet to receive
	 * \retval  0 if the packet was received
	 * \retval -1 otherwise
	 */
	int receivePacket(char *pkt_buf, size_t pkt_buf_len, size_t *pkt_len);

	/*! \brief Calculate 8-bit checksum of a buffer
	 *
	 * \param buf Pointer to buffer
	 * \param len Size of buffer
	 * \return 8-bit checksum of buffer
	 */
	static int checksum(const char *buf, size_t len);

	/*! \brief Receive acknowledgment for a packet
	 *
	 * \retval  0 received [positive] acknowledgment (ACK, `+`)
	 * \retval  1 received negative acknowledgment (NACK, `-`)
	 * \retval -1 otherwise (invalid character)
	 */
	int receiveAck(void);

	/*! \brief Create and send an OK packet (`OK`)
	 *
	 * \return Status code returned from \ref sendPacket
	 */
	int sendOkPacket();

	/*! \brief Create and send a signal packet (`S [int]`)
	 *
	 * \param buf Pointer to buffer
	 * \param buf_len Size of buffer
	 * \param signal Interrupt vector
	 * \return Status code returned from \ref sendPacket
	 */
	int sendSignalPacket(char *buf, size_t buf_len, char signal);

	/*! \brief Create and send an error packet (`E [code]`)
	 *
	 * \param buf Pointer to buffer
	 * \param buf_len Size of buffer
	 * \param error Error code
	 * \return Status code returned from \ref sendPacket
	 */
	int sendErrorPacket(char *buf, size_t buf_len, char error);

	/*! \brief Read contents of system memory area into the buffer
	 *
	 * \param buf Pointer to buffer
	 * \param buf_len Size of buffer
	 * \param addr Base address of system memory to read from
	 * \param len Size of memory area to read from
	 * \param hex Buffer content has hexadecimal (`true`) or binary (`false`) encoding
	 * \return Number of bytes read or `-1` on error (buffer to small)
	 */
	static int memRead(char *buf, size_t buf_len, uintptr_t addr, size_t len, bool hex);

	/*! \brief Write buffer contents to system memory area
	 *
	 * \param buf Pointer to buffer
	 * \param buf_len Size of buffer
	 * \param addr Base address of system memory to write at
	 * \param len Size of memory area to write at
	 * \param hex Buffer content has hexadecimal (`true`) or binary (`false`) encoding
	 * \return Number of written bytes or `-1` on error (buffer to big)
	 */
	static int memWrite(const char *buf, size_t buf_len, uintptr_t addr, size_t len, bool hex);

	/*! \brief Continue program execution (at current instruction pointer)
	 */
	static void sysContinue();

	/*! \brief Single step the next instruction.
	 */
	static void sysStep();
};
