/*
 *  atomic.h
 *  Chatter
 *
 *  Created by Curtis Jones on 2008.11.06.
 *  Copyright 2008 Curtis Jones. All rights reserved.
 *
 */

#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#ifdef __APPLE__
#  include <libkern/OSAtomic.h>
#  define ATOMIC_INC32(x) OSAtomicIncrement32(x)
#  define ATOMIC_DEC32(x) OSAtomicDecrement32(x)
#  define ATOMIC_INC32_BARRIER(x) OSAtomicIncrement32Barrier(x)
#  define ATOMIC_DEC32_BARRIER(x) OSAtomicDecrement32Barrier(x)
#  define ATOMIC_INC64_BARRIER(x) OSAtomicIncrement64Barrier(x)
#  define ATOMIC_DEC64_BARRIER(x) OSAtomicDecrement64Barrier(x)
#  define ATOMIC_ENQ(x,y,z) OSAtomicEnqueue(x,y,z)
#  define ATOMIC_DEQ(x,y) OSAtomicDequeue(x,y)
#  define ATOMIC_ADD64(x,y) OSAtomicAdd64(x,y)
#  define ATOMIC_CAS32_BARRIER(x,y,z) OSAtomicCompareAndSwap32Barrier(x,y,z)
#  define ATOMIC_CAS64_BARRIER(x,y,z) OSAtomicCompareAndSwap64Barrier(x,y,z)
#  define ATOMIC_CASPTR_BARRIER(x,y,z) OSAtomicCompareAndSwapPtrBarrier(x,y,z)

// http://gcc.gnu.org/onlinedocs/gcc-4.4.1/gcc/Atomic-Builtins.html
#elif __linux
#  define ATOMIC_INC32(x) __sync_fetch_and_add(x,1)
#  define ATOMIC_DEC32(x) __sync_fetch_and_sub(x,1)
#  define ATOMIC_INC32_BARRIER(x) __sync_fetch_and_add(x,1)
#  define ATOMIC_DEC32_BARRIER(x) __sync_fetch_and_sub(x,1)
#  define ATOMIC_INC64_BARRIER(x) __sync_fetch_and_add(x,1)
#  define ATOMIC_DEC64_BARRIER(x) __sync_fetch_and_sub(x,1)
#  define ATOMIC_ENQ(x,y,z)
#  define ATOMIC_DEQ(x,y)
#  define ATOMIC_ADD64(x,y) __sync_fetch_and_add(y,x)
#  define ATOMIC_CAS32_BARRIER(x,y,z) __sync_bool_compare_and_swap(z,x,y)
#  define ATOMIC_CAS64_BARRIER(x,y,z) __sync_bool_compare_and_swap(z,x,y)
#  define ATOMIC_CASPTR_BARRIER(x,y,z)

#else
#  error "No atomic support for this OS. Sorry."

#endif

#endif /* __ATOMIC_H__ */
