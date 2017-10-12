#ifndef __IA64_DEFNS_H__
#define __IA64_DEFNS_H__

//------------------------------------------------------------------------------
// File    : ia64_defs.h
// Author  : Ms.Moran Tzafrir
// Written : 13 April 2009
// 
// Multi-Platform C++ framework
//
// Copyright (C) 2009 Moran Tzafrir.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License 
// along with this program; if not, write to the Free Software Foundation
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//------------------------------------------------------------------------------

#include <pthread.h>
#include <sched.h>

#ifndef IA64
#define IA64
#endif

#define CACHE_LINE_SIZE 64

/*
 * I. Compare-and-swap.
 */

#define CAS32(_a, _o, _n)						\
({ __typeof__(_o) __o = _o;					\
   __asm__ __volatile__("mov ar.ccv=%0 ;;" :: "rO" (_o)); 	\
   __asm__ __volatile__("cmpxchg4.acq %0=%1,%2,ar.ccv ;; " 	\
       : "=r" (__o), "=m" (*(_a))				\
       :  "r"(_n));				\
   __o;								\
})

#define CAS64(_a, _o, _n)						\
({ __typeof__(_o) __o = _o;					\
   __asm__ __volatile__("mov ar.ccv=%0 ;;" :: "rO" (_o)); 	\
   __asm__ __volatile__("cmpxchg8.acq %0=%1,%2,ar.ccv ;; " 	\
       : "=r" (__o), "=m" (*(_a))		      		\
       :  "r"(_n));				\
   __o;								\
})

#define FAS32(_a, _n)						\
({ __typeof__(_n) __o;                                          \
   __asm__ __volatile__("xchg4 %0=%1,%2 ;; " 	\
       : "=r" (__o), "=m" (*(_a))	      			\
       :  "r"(_n));				\
   __o;								\
})

#define FAS64(_a, _n)						\
({ __typeof__(_n) __o;                                          \
   __asm__ __volatile__("xchg8 %0=%1,%2 ;; " 	\
       : "=r" (__o), "=m" (*(_a))	       			\
       :  "r"(_n));				\
   __o;								\
})

#define CAS(_x,_o,_n) ((sizeof (*_x) == 4)?CAS32(_x,_o,_n):CAS64(_x,_o,_n))
#define FAS(_x,_n)    ((sizeof (*_x) == 4)?FAS32(_x,_n)   :FAS64(_x,_n))

/* Update Integer location, return Old value. */
#define CASIO CAS
#define FASIO FAS
/* Update Pointer location, return Old value. */
#define CASPO CAS64
#define FASPO FAS64
/* Update 32/64-bit location, return Old value. */
#define CAS32O CAS32
#define CAS64O CAS64


/*
 * II. Memory barriers. 
 *  WMB(): All preceding write operations must commit before any later writes.
 *  RMB(): All preceding read operations must commit before any later reads.
 *  MB():  All preceding memory accesses must commit before any later accesses.
 * 
 *  If the compiler does not observe these barriers (but any sane compiler
 *  will!), then VOLATILE should be defined as 'volatile'.
 */

#define MB()  __asm__ __volatile__ (";; mf ;; " : : : "memory")
#define WMB() MB()
#define RMB() MB()
#define VOLATILE /*volatile*/

/*
 * III. Cycle counter access.
 */

typedef unsigned long long tick_t;
#define RDTICK() \
    ({ tick_t __t; __asm__ __volatile__ ("mov %0=ar.itc ;;" : "=rO" (__t)); __t; })



/*
 * IV. Types.
 */

typedef unsigned char      _u8;
typedef unsigned short     _u16;
typedef unsigned int       _u32;
typedef unsigned long long _u64;

#endif /* __IA64_DEFNS_H__ */
