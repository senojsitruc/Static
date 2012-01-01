/*
 *  chebyshev.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  A recursion based filter.
 *
 *  http://www.dspguide.com/ch20/4.htm
 *  http://www.purebits.com/appnote9.html
 *  http://en.wikipedia.org/wiki/Chebyshev_filter
 *
 *  a[] = denominator coefficients
 *  b[] = numerator coefficients
 */

#ifndef __CHEBYSHEV_H__
#define __CHEBYSHEV_H__

#include "../filter.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// chebyshev_type
//
typedef enum
{
	CHEBYSHEV_LOW_PASS  = (1 << 0),      // low pass
	CHEBYSHEV_HIGH_PASS = (1 << 1)       // high pass
} chebyshev_type;

//
// chebyshev
//
struct chebyshev
{
	filter_t filter;                     // parent class
	
	double cutoff;                       // cutoff frequency (0 to 0.5)
	double ripple;                       // ripple percent
	double poles;                        // number of poles
	chebyshev_type type;                 // low or high pass
	
	double coef_a[23];                   // 'a' coefficients
	double coef_b[23];                   // 'b' coefficients
};
typedef struct chebyshev chebyshev_t;





/**
 * chebyshev, cutoff frequency (0 to 0.5 - which is a percent of the sampling frequency), low/high 
 * pass, ripple percent (0 - 29), poles (even number between 2 - 20), maximum output signal size, 
 * pool
 */
int chebyshev_init (chebyshev_t*, double, chebyshev_type, double, uint32_t, uint32_t, opool_t*);

/**
 *
 */
int chebyshev_destroy (chebyshev_t*);





/**
 *
 */
int chebyshev_feed (chebyshev_t*, uint32_t*, double*);

/**
 *
 */
int chebyshev_reset (chebyshev_t*);





/**
 *
 */
chebyshev_t* chebyshev_retain (chebyshev_t*);

/**
 *
 */
void chebyshev_release (chebyshev_t*);

#endif /* __CHEBYSHEV_H__ */
