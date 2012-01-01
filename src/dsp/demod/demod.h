/*
 *  demod.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.29.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __DSP_DEMOD_H__
#define __DSP_DEMOD_H__

#include "../dsp.h"
#include "../../misc/mem/cobject.h"
#include "../../misc/mem/opool.h"

//
// demod
//
struct demod
{
	dsp_t dsp;												// parent class
	
	/* derived class overloads */
	__dsp_feed_func __feed;
	__dsp_reset_func __reset;
};
typedef struct demod demod_t;





/**
 * demod, name, pool
 */
int demod_init (demod_t*, char*, cobject_destroy_func, opool_t*);

/**
 *
 */
int demod_destroy (demod_t*);





/**
 *
 */
int demod_feed (demod_t*, uint32_t*, double*);

/**
 *
 */
int demod_reset (demod_t*);





/**
 *
 */
demod_t* demod_retain (demod_t*);

/**
 *
 */
void demod_release (demod_t*);

#endif /* __DSP_DEMOD_H__ */
