/*! \file
 *  \brief General purpose \ref string "String functions"
 */
#pragma once

#include "types.h"

/*! \defgroup string String function
 *	\brief String functions as provided by `%string.h` in the C standard library
 */

/*! \brief Find the first occurrence of a character in a string
 * \ingroup string
 * \param s string to
 * \param c character to find
 * \return Pointer to first occurrence of the character
 *         or to null byte at the end of the string if not found
 */
extern "C" char *strchrnul(const char *s, int c);

/*! \brief Find the first occurrence of a character in a string
 * \ingroup string
 * \param s string to
 * \param c character to find
 * \return Pointer to first occurrence of the character
 *         or to nullptr if not found
 */
extern "C" char *strchr(const char *s, int c);

/*! \brief Compare two strings
 * \ingroup string
 * \param s1 first string
 * \param s2 second string
 * \return an integer less than, equal to, or greater than zero if first string is found, respectively,
 *         to be less than, to match, or be greater than second string
 */
extern "C" int strcmp(const char *s1, const char *s2);

/*! \brief Compare two strings
 * \ingroup string
 * \param s1 first string
 * \param s2 second string
 * \param n number of bytes to compare
 * \return an integer less than, equal to, or greater than zero if the given number of bytes of the first string are
 *          found, respectively, to be less than, to match, or be greater than second string
 */
extern "C" int strncmp(const char *s1, const char *s2, size_t n);

/*! \brief Calculate the length of a string
 * \ingroup string
 * \param s pointer to a string
 * \return number of bytes in the string
 */
extern "C" size_t strlen(const char *s);

/*! \brief Calculate the length of a string, limited by maxlen
 * \ingroup string
 * \param s pointer to a string
 * \param maxlen upper limit of length to be returned
 * \return number of bytes in the string, or maxlen -- whichever is smaller
 */
extern "C" size_t strnlen(const char *s, size_t maxlen);

/*! \brief Copy the contents of a string
 * including the terminating null byte (`\0`)
 * \ingroup string
 * \param dest destination string buffer
 * \param src source string buffer
 * \return a pointer to the destination string buffer
 * \note Beware of buffer overruns!
 */
extern "C" char * strcpy(char * dest, const char * src);  //NOLINT

/*! \brief Copy the contents of a string up to a maximum length
 * or the terminating null byte (`\0`), whatever comes first.
 * \ingroup string
 * \param dest destination string buffer
 * \param src source string buffer
 * \param n maximum number of bytes to copy
 * \return a pointer to the destination string buffer
 * \note If there is no null byte (`\0`) among the first `n` bytes, the destination will not be null-terminated!
 */
extern "C" char * strncpy(char * dest, const char * src, size_t n);

/*! \brief Copy a memory area
 * \ingroup string
 * \param dest destination buffer
 * \param src source buffer
 * \param size number of bytes to copy
 * \return pointer to destination
 * \note The memory must not overlap!
 */
extern "C" void* memcpy(void * __restrict__ dest, void const * __restrict__ src, size_t size);

/*! \brief Copy a memory area
 * while the source may overlap with the destination
 * \ingroup string
 * \param dest destination buffer
 * \param src source buffer
 * \param size number of bytes to copy
 * \return pointer to destination
 */
extern "C" void* memmove(void * dest, void const * src, size_t size);

/*! \brief Fill a memory area with a pattern
 * \ingroup string
 * \param dest destination buffer
 * \param pattern single byte pattern
 * \param size number of bytes to fill with pattern
 * \return pointer to destination
 */
extern "C" void* memset(void *dest, int pattern, size_t size);

/*! \brief Compare a memory area
 * \ingroup string
 * \param s1 first memory buffer
 * \param s2 second memory buffer
 * \param n number of bytes to compare
 * \return an integer less than, equal to, or greater than zero if the first n bytes of s1 is found, respectively,
 *         to be less than, to match, or be greater than the first n bytes of s2.
 */
extern "C" int memcmp(const void * s1, const void * s2, size_t n);
