/*! \file
 *  \brief Communication via the \ref Serial interface (RS-232)
 */

#pragma once

#include "types.h"

/*! \brief Serial interface.
 *  \ingroup io
 *
 *  This class provides a serial interface (COM1 - COM4) for communication with the outside world.
 *
 *  The first IBM PC used the external chip [8250 UART](https://de.wikipedia.org/wiki/NSC_8250), whereas, in today's
 *  systems, this functionality is commonly integrated into the southbridge, but remained compatible.
 *
 * \see [PC8250A Data Sheet](uart-8250a.pdf#page=11) (Registers on page 11)
 * \see [PC16550D Data Sheet](uart-16550d.pdf#page=16) (Successor, for optional FIFO buffer, page 16)
 */

class Serial {
 public:
	/*! \brief COM-Port
	 *
	 *  The serial interface and its hardware addresses. Modern desktop PCs have, at most,
	 *  a single, physical COM-port (`COM1`)
	 */
	enum ComPort{
		COM1 = 0x3f8,
		COM2 = 0x2f8,
		COM3 = 0x3e8,
		COM4 = 0x2e8,
	};

	/*! \brief Transmission speed
	 *
	 *  The unit Baud describes the transmission speed in number of symbols per seconds.
	 *  1 Baud therefore equals the transmission of 1 symbol per second.
	 *  The possible Baud rates are whole-number dividers of the clock frequency of 115200 Hz..
	 */
	enum BaudRate {
		BAUD_300    = 384,
		BAUD_600    = 192,
		BAUD_1200   =  96,
		BAUD_2400   =  48,
		BAUD_4800   =  24,
		BAUD_9600   =  12,
		BAUD_19200  =   6,
		BAUD_38400  =   3,
		BAUD_57600  =   2,
		BAUD_115200 =   1,
	};

	/*! \brief Number of data bits per character */
	enum DataBits {
		DATA_5BIT = 0,
		DATA_6BIT = 1,
		DATA_7BIT = 2,
		DATA_8BIT = 3,
	};

	/*! \brief Number of stop bits per character */
	enum StopBits {
		STOP_1BIT   = 0,
		STOP_1_5BIT = 4,
		STOP_2BIT   = 4,
	};

	/*! \brief parity bit */
	enum Parity {
		PARITY_NONE  =  0,
		PARITY_ODD   =  8,
		PARITY_EVEN  = 24,
		PARITY_MARK  = 40,
		PARITY_SPACE = 56,
	};

 private:
	/*! \brief register index */
	enum RegisterIndex {
		// if Divisor Latch Access Bit [DLAB] = 0
		RECEIVE_BUFFER_REGISTER   = 0,  ///< read only
		TRANSMIT_BUFFER_REGISTER  = 0,  ///< write only
		INTERRUPT_ENABLE_REGISTER = 1,

		// if Divisor Latch Access Bit [DLAB] = 1
		DIVISOR_LOW_REGISTER      = 0,
		DIVISOR_HIGH_REGISTER     = 1,

		// (irrespective from DLAB)
		INTERRUPT_IDENT_REGISTER  = 2,  ///< read only
		FIFO_CONTROL_REGISTER     = 2,  ///< write only -- 16550 and newer (esp. not 8250a)
		LINE_CONTROL_REGISTER     = 3,  ///< highest-order bit is DLAB (see above)
		MODEM_CONTROL_REGISTER    = 4,
		LINE_STATUS_REGISTER      = 5,
		MODEM_STATUS_REGISTER     = 6
	};

	/*! \brief Mask for the respective register */
	enum RegisterMask : uint8_t {
		// Interrupt Enable Register
		RECEIVED_DATA_AVAILABLE            = 1 << 0,
		TRANSMITTER_HOLDING_REGISTER_EMPTY = 1 << 1,
		RECEIVER_LINE_STATUS               = 1 << 2,
		MODEM_STATUS                       = 1 << 3,

		// Interrupt Ident Register
		INTERRUPT_PENDING   = 1 << 0,  ///< 0 means interrupt pending
		INTERRUPT_ID_0      = 1 << 1,
		INTERRUPT_ID_1      = 1 << 2,

		// FIFO Control Register
		ENABLE_FIFO         = 1 << 0,  ///< 0 means disabled ^= conforming to 8250a
		CLEAR_RECEIVE_FIFO  = 1 << 1,
		CLEAR_TRANSMIT_FIFO = 1 << 2,
		DMA_MODE_SELECT     = 1 << 3,
		TRIGGER_RECEIVE     = 1 << 6,

		// Line Control Register
		                                    //  bits per character:  5   6   7   8
		WORD_LENGTH_SELECT_0     = 1 << 0,  //  Setting Select0:     0   1   0   1
		WORD_LENGTH_SELECT_1     = 1 << 1,  //  Setting Select1:     0   0   1   1
		NUMBER_OF_STOP_BITS      = 1 << 2,  //  0 ≙ one stop bit, 1 ≙ 1.5/2 stop bits
		PARITY_ENABLE            = 1 << 3,
		EVEN_PARITY_SELECT       = 1 << 4,
		STICK_PARITY             = 1 << 5,
		SET_BREAK                = 1 << 6,
		DIVISOR_LATCH_ACCESS_BIT = 1 << 7,  // DLAB

		// Modem Control Register
		DATA_TERMINAL_READY = 1 << 0,
		REQUEST_TO_SEND     = 1 << 1,
		OUT_1               = 1 << 2,
		OUT_2               = 1 << 3,  // must be set for interrupts!
		LOOP                = 1 << 4,

		// Line Status Register
		DATA_READY                   = 1 << 0,  // Set when there is a value in the receive buffer
		OVERRUN_ERROR                = 1 << 1,
		PARITY_ERROR                 = 1 << 2,
		FRAMING_ERROR                = 1 << 3,
		BREAK_INTERRUPT              = 1 << 4,
		TRANSMITTER_HOLDING_REGISTER = 1 << 5,
		TRANSMITTER_EMPTY            = 1 << 6,  // Send buffer empty (ready to send)

		// Modem Status Register
		DELTA_CLEAR_TO_SEND          = 1 << 0,
		DELTA_DATA_SET_READY         = 1 << 1,
		TRAILING_EDGE_RING_INDICATOR = 1 << 2,
		DELTA_DATA_CARRIER_DETECT    = 1 << 3,
		CLEAR_TO_SEND                = 1 << 4,
		DATA_SET_READY               = 1 << 5,
		RING_INDICATOR               = 1 << 6,
		DATA_CARRIER_DETECT          = 1 << 7
	};

	/*! \brief Read value from register
	 *
	 *  \opt Implement Method
	 *
	 *  \param reg Register index
	 *  \return The value read from register
	 */
	char readReg(RegisterIndex reg);

	/*! \brief Write value to register
	 *
	 *  \opt Implement Method
	 *
	 *  \param reg Register index
	 *  \param out value to be written
	 */
	void writeReg(RegisterIndex reg, char out);

 protected:
	/*! \brief Selected COM port */
	const ComPort port;

 public:
	/*! \brief Constructor
	 *
	 * Creates a Serial object that encapsulates the used COM port, as well as the parameters used for the
	 * serial connection. Default values are `8N1` (8 bit, no parity bit, one stop bit) with 115200 Baud using COM1.
	 *
	 *  \opt Implement Constructor
	 */
	explicit Serial(ComPort port = COM1, BaudRate baud_rate = BAUD_115200, DataBits data_bits = DATA_8BIT,
	                StopBits stop_bits = STOP_1BIT, Parity parity = PARITY_NONE);

	/*! \brief Read one byte from the serial interface
	 *
	 *  \opt Implement Method
	 *
	 *  \param blocking If set, \ref read() blocks until one byte was read
	 *  \return Value read from serial interface (or `-1 ` if non-blocking and no data ready)
	 */
	int read(bool blocking = true);

	/*! \brief Write one byte to the serial interface
	 *
	 *  \opt Implement Method
	 *
	 *  \param out Byte to be written
	 *  \param blocking If set, \ref write() blocks until the byte was written
	 *  \return Byte written (or `-1` if writing byte failed)
	 */
	int write(char out, bool blocking = true);

};
