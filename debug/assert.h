/*! \file
 *  \brief Contains several macros usable for making assertions
 *
 *  Depending on the type of assertion (either static or at runtime), a failing assertion will trigger an error.
 *  For static assertion, this error will be shown at compile time and abort compilation.
 *  Runtime assertions will trigger a message containing details about the error occurred and will make the CPU die.
 */

#pragma once

#ifndef STRINGIFY
/*! \def STRINGIFY(S)
 *  \brief Converts a macro parameter into a string
 *  \ingroup debug
 *  \param S Expression to be converted
 *  \return stringified version of S
 */
#define STRINGIFY(S) #S
#endif

/*! \def assert_size(TYPE, SIZE)
 *  \brief Statically ensure (at compile time) that a data type (or variable) has the expected size.
 *  \ingroup debug
 *  \param TYPE The type to be checked
 *  \param SIZE Expected size in bytes
 */
#define assert_size(TYPE, SIZE) \
	static_assert(sizeof(TYPE) == (SIZE), "Wrong size for " STRINGIFY(TYPE))

/*! \def assert(EXP)
 *  \brief Ensure (at execution time) an expression evaluates to `true`, print an error message and stop the CPU otherwise.
 *  \ingroup debug
 *  \param EXP The expression to be checked
 */
#ifdef NDEBUG
#define assert(EXP) \
	do { \
		(void)sizeof(EXP); \
	} while (false)
#else
#define assert(EXP) \
	do { \
		if (__builtin_expect(!(EXP), 0)) { \
			assertion_failed(STRINGIFY(EXP), __func__, __FILE__, __LINE__); \
		} \
	} while (false)

/*! \brief Handles a failed assertion
 *
 *  This function will print a message containing further information about the
 *  failed assertion and stops the current CPU permanently.
 *
 *  \note This function should never be called directly, but only via the macro `assert`.
 *
 *  \todo Implement Remainder of Method (output & CPU stopping)
 *
 *  \param exp  Expression that did not hold
 *  \param func Name of the function in which the assertion failed
 *  \param file Name of the file in which the assertion failed
 *  \param line Line in which the assertion failed
 */
[[noreturn]] void assertion_failed(const char * exp, const char * func, const char * file, int line);
#endif
