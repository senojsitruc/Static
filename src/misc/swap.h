/*
 *  swap.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------
 *
 */

#ifndef __SWAP_H__
#define __SWAP_H__

#include <arpa/inet.h>

#define __swap16(x)										\
	((uint16_t)(												\
		(((uint16_t)(x) & 0xff00) >> 8) | \
		(((uint16_t)(x) & 0x00ff) << 8)		\
	))

#define __swap32(x)													 \
	((uint32_t)(														 \
		(((uint32_t)(x) & 0xff000000) >> 24) | \
		(((uint32_t)(x) & 0x00ff0000) >>  8) | \
		(((uint32_t)(x) & 0x0000ff00) <<  8) | \
		(((uint32_t)(x) & 0x000000ff) << 24)	 \
	))

#define __swap64(x)																		\
	((uint64_t)(																				\
		(((uint64_t)(x) & 0xff00000000000000ULL) >> 56) | \
		(((uint64_t)(x) & 0x00ff000000000000ULL) >> 40) | \
		(((uint64_t)(x) & 0x0000ff0000000000ULL) >> 24) | \
		(((uint64_t)(x) & 0x000000ff00000000ULL) >>  8) | \
		(((uint64_t)(x) & 0x00000000ff000000ULL) <<  8) | \
		(((uint64_t)(x) & 0x0000000000ff0000ULL) << 24) | \
		(((uint64_t)(x) & 0x000000000000ff00ULL) << 40) | \
		(((uint64_t)(x) & 0x00000000000000ffULL) << 56)		\
	))

#if BYTE_ORDER == BIG_ENDIAN
#  define swap16(x) (x)
#  define swap32(x) (x)
#  define swap64(x) (x)
#elif BYTE_ORDER == LITTLE_ENDIAN
#  define swap16(x)											\
		((uint16_t)(												\
			(((uint16_t)(x) & 0xff00) >> 8) | \
			(((uint16_t)(x) & 0x00ff) << 8)		\
		))
#  define swap32(x)													 \
		((uint32_t)(														 \
			(((uint32_t)(x) & 0xff000000) >> 24) | \
			(((uint32_t)(x) & 0x00ff0000) >>  8) | \
			(((uint32_t)(x) & 0x0000ff00) <<  8) | \
			(((uint32_t)(x) & 0x000000ff) << 24)	 \
		))
#  define swap64(x)																			\
		((uint64_t)(																				\
			(((uint64_t)(x) & 0xff00000000000000ULL) >> 56) | \
			(((uint64_t)(x) & 0x00ff000000000000ULL) >> 40) | \
			(((uint64_t)(x) & 0x0000ff0000000000ULL) >> 24) | \
			(((uint64_t)(x) & 0x000000ff00000000ULL) >>  8) | \
			(((uint64_t)(x) & 0x00000000ff000000ULL) <<  8) | \
			(((uint64_t)(x) & 0x0000000000ff0000ULL) << 24) | \
			(((uint64_t)(x) & 0x000000000000ff00ULL) << 40) | \
			(((uint64_t)(x) & 0x00000000000000ffULL) << 56)		\
		))
#else
#error "Could not determine endianess!"
#endif

#if BYTE_ORDER == BIG_ENDIAN
#  define swap16be(x) (x)
#  define swap32be(x) (x)
#  define swap64be(x) (x)
#  define swap16le(x) __swap16(x)
#  define swap32le(x) __swap32(x)
#  define swap64le(x) __swap64(x)
#elif BYTE_ORDER == LITTLE_ENDIAN
#  define swap16be(x) __swap16(x)
#  define swap32be(x) __swap32(x)
#  define swap64be(x) __swap64(x)
#  define swap16le(x) (x)
#  define swap32le(x) (x)
#  define swap64le(x) (x)
#endif

#endif /* __SWAP_H__ */
