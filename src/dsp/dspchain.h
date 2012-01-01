/*
 *  dspchain.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  A chain of dsp objects for signal processing.
 *
 */

#ifndef __DSP_CHAIN_H__
#define __DSP_CHAIN_H__

#include "dsp.h"
#include "../misc/mem/cobject.h"
#include "../misc/mem/opool.h"

/**
 * Used in dspchain_add(). Indicates that the dsp should be placed in the next available slot in the
 * dspchain.
 */
#define DSPCHAIN_NEXT_POS -1

//
// dspchain
//
struct dspchain
{
	cobject_t cobject;								// parent class
	
	dsp_t **dsp_list;									// signal processing
	uint32_t dsp_size;								// list size
	uint32_t dsp_count;								// list item count
};
typedef struct dspchain dspchain_t;





/**
 * dspchain, size, pool
 */
int dspchain_init (dspchain_t*, uint32_t, opool_t*);

/**
 *
 */
int dspchain_destroy (dspchain_t*);





/**
 *
 */
int dspchain_add (dspchain_t*, dsp_t*, int32_t);

/**
 *
 */
int dspchain_del (dspchain_t*, uint32_t);

/**
 *
 */
int dspchain_get (dspchain_t*, uint32_t, dsp_t**);

/**
 *
 */
int dspchain_cnt (dspchain_t*, uint32_t*);

/**
 *
 */
int dspchain_run (dspchain_t*, uint32_t*, void*);

/**
 *
 */
int dspchain_reset (dspchain_t*);





/**
 *
 */
dspchain_t* dspchain_retain (dspchain_t*);

/**
 *
 */
void dspchain_release (dspchain_t*);

#endif /* __DSP_CHAIN_H__ */
