/*! \file
 *  \brief This file contains the \ref OutputStream
 *
 *  Along with the class OutputStream itself, this file contains definitions for the
 *  manipulators \ref hex, \ref dec, \ref oct, and \ref bin, which are used for
 *  changing the radix, and \ref endl for signaling the end of the current line.
 *  \ingroup io
 *
 *  \par Manipulators
 *  To simplify formatting text and numbers using the class OutputStream, we define
 *  so-called manipulators.
 *  For example, the expression
 *  <tt>kout << "a = " << dec << a << " is hexadecimal " << hex << a << endl;</tt>
 *  should, at first, print the value stored in decimal and then in hexadecimal
 *  form, followed by a line break.
 *  The intended properties can be realized by implementing \ref hex, \ref dec, \ref oct, \ref bin,
 *  and \ref endl as functions (i.e., they are, in particular, not methods of \ref OutputStream)
 *  that take (as first parameter) and return a reference to an OutputStream object.
 *  When compiling the expression show above, the method
 *  <tt>OutputStream& OutputStream::operator<< ((*f*) (OutputStream&))</tt>
 *  is chosen when one of the functions \ref hex, \ref dec, \ref oct, \ref bin, or \ref endl
 *  is streamed an \ref OutputStream, which finally will execute the passed function.
 *
 *  \note The term manipulator originates from the book
 *        [The C++ Programming Language](http://www.stroustrup.com/4th.html)
 *        by Bjarne Stroustrup. Refer to this book for further explanations.
 */

#pragma once

#include "object/stringbuffer.h"

/*! \brief The class OutputStream corresponds, essentially, to the class ostream
 *  from the C++ IO-Stream library.
 *
 *  As relying on the method \ref Stringbuffer::put() is quite cumbersome when
 *  not only printing single characters, but numbers and whole strings, the
 *  class OutputStream provides a convenient way of composing output of variables of
 *  varying data types.
 *  Therefore, OutputStream implements shift operators `operator<<`` for various
 *  data types (similar to those known from the C++ IO-Stream library)
 *
 *  For further convenience, OutputStream also allows printing integral numbers in
 *  decimal, binary, octal, and hexadecimal format.
 *  Remember that, for negative numbers, the sign is only printed when using the
 *  decimal number system; for binary, octal, and hex, the number is printed as
 *  stored in the machine word without interpreting the sign.
 *  For Intel CPUs, two's complement is used for storing negative values, `-1`,
 *  for example, will print hex `FFFFFFFF` and octal `37777777777`.
 *
 *  OutputStream's public methods/operators all return a reference to the object
 *  they are called on (i.e. `*this`). Returning `*this` allows chaining those
 *  stream operators in a single expression, such as
 *  <tt>kout << "a = " << a</tt>;
 *
 *  At this point in time, OutputStream implements `operator<<` for chars, strings
 *  and whole numbers. An additional `operator<<` allows using manipulators
 *  whose detailed description is given below.
 */

class OutputStream : public Stringbuffer {
	OutputStream(const OutputStream&)            = delete;
	OutputStream& operator=(const OutputStream&) = delete;

 private:
	OutputStream& helper(unsigned long long ival, bool sign);

 public:
	/*! \brief Number system used for printing integral numbers (one of 2,
	 *  8, 10, or 16)
	 */
	int base;

	/*! \brief Default constructor. Initial number system is decimal.
	 *
	 *  \todo Implement Constructor
	 *
	 */
	OutputStream() {}

	/*! \brief Destructor
	 */
	virtual ~OutputStream() {}

	/*! \brief Clears the buffer.
	 *
	 *  Pure virtual method that must be implemented by derived
	 *  (non-abstract) classes.
	 *  Formatting of the buffer contents can be implemented differently by
	 *  different derived classes
	 */
	virtual void flush() = 0;

	/*! \brief Print a single character
	 *
	 *  \todo Implement Operator
	 *
	 *  \param c Character to be printed
	 *  \return Reference to OutputStream os; allows operator chaining.
	 */
	OutputStream& operator << (char c);

	/*! \brief Print a single character
	 *  \note In C, there are no "characters" in that sense, but only
	 *  integers. A `char`, therefore, is a 8 bit number with the most
	 *  significant bit (optionally) representing a sign.
	 *  Depending on whether signed or not, the value ranges are [-128, 127]
	 *  or [0; 255]. For GCC, a `char` is a `signed char`.
	 *
	 *  \todo Implement Operator
	 *
	 *  \param c Character to be printed
	 *  \return Reference to OutputStream os; allows operator chaining.
	 */
	OutputStream& operator << (unsigned char c);

	/*! \brief Printing a null-terminated string
	 *
	 *  \todo Implement Operator
	 *
	 *  \param string String to be printed
	 *  \return Reference to OutputStream os; allows operator chaining.
	 */
	OutputStream& operator << (const char* string);

	/*! \brief Print a boolean value
	 *
	 *  \todo Implement Operator
	 *
	 *  \param b Boolean to be printed
	 *  \return Reference to OutputStream os; allows operator chaining.
	 */
	OutputStream& operator << (bool b);

	/*! \brief Print an integral number in radix `base`
	 *
	 *  \todo Implement Operator
	 *
	 *  \param ival Number to be printed
	 *  \return Reference to OutputStream os; allows operator chaining.
	 */
	OutputStream& operator << (short ival);

	/// \copydoc OutputStream::operator<<(short)
	OutputStream& operator << (unsigned short ival);

	/// \copydoc OutputStream::operator<<(short)
	OutputStream& operator << (int ival);

	/// \copydoc OutputStream::operator<<(short)
	OutputStream& operator << (unsigned int ival);

	/// \copydoc OutputStream::operator<<(short)
	OutputStream& operator << (long ival);

	/// \copydoc OutputStream::operator<<(short)
	OutputStream& operator << (unsigned long ival);

	/// \copydoc OutputStream::operator<<(short)
	OutputStream& operator << (long long ival);

	/// \copydoc OutputStream::operator<<(short)
	OutputStream& operator << (unsigned long long ival);

	/*! \brief Print a pointer as hexadecimal number
	 *
	 *  \todo Implement Operator
	 *
	 *  \param ptr Pointer to be printed
	 *  \return Reference to OutputStream os; allows operator chaining.
	 */
	OutputStream& operator << (const void* ptr);

	/*! \brief Calls one of the manipulator functions.
	 *
	 *  Method that calls the manipulator functions defined below, which
	 *  allow modifying the stream's behavior by, for instance, changing the
	 *  number system.
	 *
	 *  \todo Implement Operator
	 *
	 *  \param f Manipulator function to be called
	 *  \return Reference to OutputStream os; allows operator chaining.
	 */
	OutputStream& operator << (OutputStream& (*f) (OutputStream&));
};

/*! \brief Enforces a buffer flush.
 *
 *  \todo Implement Manipulator
 *
 *  \param os Reference to stream to be flushed.
 *  \return Reference to OutputStream os; allows operator chaining.
 */
OutputStream& flush(OutputStream& os);

/*! \brief Prints a newline character to the stream and issues a buffer flush.
 *
 *  \todo Implement Manipulator
 *
 *  \param os Reference to stream to be modified.
 *  \return Reference to OutputStream os; allows operator chaining.
 */
OutputStream& endl(OutputStream& os);

/*! \brief Print subsequent numbers in binary form.
 *
 *  \todo Implement Manipulator
 *
 *  \param os Reference to stream to be modified.
 *  \return Reference to OutputStream os; allows operator chaining.
 */
OutputStream& bin(OutputStream& os);

/*! \brief Print subsequent numbers in octal form.
 *
 *  \todo Implement Manipulator
 *
 *  \param os Reference to stream to be modified.
 *  \return Reference to OutputStream os; allows operator chaining.
 */
OutputStream& oct(OutputStream& os);

/*! \brief Print subsequent numbers in decimal form.
 *
 *  \todo Implement Manipulator
 *
 *  \param os Reference to stream to be modified.
 *  \return Reference to OutputStream os; allows operator chaining.
 */
OutputStream& dec(OutputStream& os);

/*! \brief Print subsequent numbers in hex form.
 *
 *  \todo Implement Manipulator
 *
 *  \param os Reference to stream to be modified.
 *  \return Reference to OutputStream os; allows operator chaining.
 */
OutputStream& hex(OutputStream& os);
