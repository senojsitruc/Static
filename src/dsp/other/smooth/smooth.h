/*
 *  smooth.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __SMOOTH_H__
#define __SMOOTH_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// smooth
//
struct smooth
{
	dsp_t dsp;												// parent class
	uint32_t period;									// smoothiness
};
typedef struct smooth smooth_t;





/**
 * smooth, smoothiness, pool
 */
int smooth_init (smooth_t*, uint32_t, opool_t*);

/**
 *
 */
int smooth_destroy (smooth_t*);





/**
 *
 */
int smooth_feed (smooth_t*, uint32_t*, double*);

/**
 *
 */
int smooth_reset (smooth_t*);





/**
 *
 */
smooth_t* smooth_retain (smooth_t*);

/**
 *
 */
void smooth_release (smooth_t*);

#endif /* __SMOOTH_H__ */
