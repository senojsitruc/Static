/*
 *  cpx2real.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.27.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------*
 *
 *  Complex-to-Real: sqrt(i^2 + q^2); also called rectangular to polar.
 *
 */

#ifndef __DSP_CPX2REAL_H__
#define __DSP_CPX2REAL_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// cpx2real
//
struct cpx2real
{
	dsp_t dsp;												// parent class
};
typedef struct cpx2real cpx2real_t;





/**
 * cpx2real, pool
 */
int cpx2real_init (cpx2real_t*, opool_t*);

/**
 *
 */
int cpx2real_destroy (cpx2real_t*);





/**
 *
 */
int cpx2real_feed (cpx2real_t*, uint32_t*, double*);

/**
 *
 */
int cpx2real_reset (cpx2real_t*);





/**
 *
 */
cpx2real_t* cpx2real_retain (cpx2real_t*);

/**
 *
 */
void cpx2real_release (cpx2real_t*);

#endif /* __DSP_CPX2REAL_H__ */
