/*! \file
 *  \brief C-style dynamic memory allocation interface
 */

#pragma once

#include "types.h"

/*! \brief Allocate a memory block
 * The memory is not initialized.
 *
 * \param size Requested size of memory in bytes.
 * \return Pointer to memory or `nullptr` on error (no memory available) or if size
 *         was zero.
 * \opt Implement method
 */
extern "C" void *malloc(size_t size);

/*! \brief Free an allocated memory block
 *
 * \param ptr Pointer to an previously allocated memory block.
 * \opt Implement method
 */
extern "C" void free(void *ptr);

/*! \brief Change the size of an allocated memory block
 * The contents will be unchanged in the range from the start of the region up
 * to the minimum of the old and new sizes. If the new size is larger than the
 * old size, the added memory will not be initialized.
 *
 * \param ptr  Pointer to an previously allocated memory block.
 *             If `nullptr`, then the call is equivalent to \ref malloc().
 * \param size New size of the memory block. If equal to zero, then the call
 *             is equivalent to free
 * \return Pointer to new memory block or `nullptr` on error
 * \opt Implement method
 */
extern "C" void *realloc(void *ptr, size_t size);

/*! \brief Allocate memory for an array of elements
 * The memory is set to zero.
 *
 * \param nmemb Number of elements
 * \param size Size of an element in bytes
 * \return pointer to the allocated memory or `nullptr` if the request fails
 * \opt Implement method
 */
extern "C" void *calloc(size_t nmemb, size_t size);
