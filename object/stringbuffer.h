/*! \file
 *  \brief \ref Stringbuffer composes single characters into a buffer
 */

#pragma once

/*! \brief The class Stringbuffer composes single characters into a longer text that can be processed on block.
 *
 *  To make Stringbuffer as versatile as possible, the class does make
 *  assumptions about neither the underlying hardware, nor the meaning of
 *  "processing". When flush() is called (i.e., either on explicit request or
 *  once the buffer is full). To be hardware independent, flush() is to be
 *  implemented by the derived classes.
 *
 *  \par Hints for Implementation
 *  Use a buffer of fixed size for caching characters, which should be
 *  accessible by derived classes.
 *  Keep in mind that the derived implementation of flush() will need to know
 *  about numbers of characters in the buffer.
 *
 *  \par Notes
 *  Reason for the existence of this class is that generating longer texts is
 *  often implemented by assembly of small fragments (such as single characters
 *  or numbers).
 *  However, writing such small fragments directly to (for example) screen is
 *  quite inefficient (e.g., due to the use of IO ports, syscalls, or locks) and
 *  can be improved drastically by delaying the output step until the assembly
 *  is finished (or the buffer runs full).
 */
class Stringbuffer {
	// Prevent copies and assignments
	Stringbuffer(const Stringbuffer&)            = delete;
	Stringbuffer& operator=(const Stringbuffer&) = delete;

	// All variables and methods are protected in this class,
	// as the derived classes need direct access to be buffer,
	// the constructor, the destructor, and the method put.
	// flush() is to be implemented either way and may be redefined
	// as public.

 protected:
	/// buffer containing characters that will be printed upon flush()
	char buffer[80];
	/// current position in the buffer
	long unsigned pos;

	/*! \brief Constructor; Marks the buffer as empty
	 *
	 *  \todo Complete Constructor
	 */
	Stringbuffer() : pos(0) {}

	/*! \brief Inserts a character into the buffer.
	 *
	 *  Once the buffer is full, a call to flush() will be issued and
	 *  thereby clearing the buffer.
	 *
	 *  \todo Implement Method
	 *
	 *  \param c Char to be added
	 */
	void put(char c);

	/*! \brief Flush the buffer contents
	 *
	 *  This method is to be defined in derived classes, as only those know
	 *  how to print characters.
	 *  flush() is required to reset the position pos.
	 */
	virtual void flush() = 0;

 public:
	/*! \brief Destructor (nothing to do here)
	 */
	virtual ~Stringbuffer() { }
};
