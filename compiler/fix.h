/*! \file
 *  \brief Compiler-dependent fixes & idiosyncrasies
 */

#pragma once

#ifdef __clang__
# define UNUSED_STRUCT_FIELD __attribute__((unused))
#else
// GCC does not understand this attribute correctly for structures
# define UNUSED_STRUCT_FIELD
#endif

#if defined(__GNUC__) && !defined(__clang__)
// Only GCC understands the error attribute
# define ERROR_ON_CALL(MSG)  __attribute__((error(MSG)));
#else
# define ERROR_ON_CALL(MSG)
#endif

#define MAYBE_UNUSED __attribute__((unused))
