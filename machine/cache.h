/*! \file
 *  \brief Helper for cache alignment
 */

#pragma once

#include "debug/assert.h"

// Helper for aligning to cache line (to prevent false sharing)
#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE    64
#endif
#define cache_aligned      alignas(CACHE_LINE_SIZE)

/*!
 *  \def assert_cache_aligned(TYPE)
 *  \brief Compile time check of cache alignment
 *  \param TYPE data type to check
 */
#define assert_cache_aligned(TYPE) \
	static_assert(sizeof(TYPE) % CACHE_LINE_SIZE == 0,  STRINGIFY(TYPE) "Not aligned on cache boundary")
