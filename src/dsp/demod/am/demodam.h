/*
 *  demodam.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.31.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __DEMOD_AM_H__
#define __DEMOD_AM_H__

#include "../demod.h"
#include "../../../misc/mem/opool.h"
#include <stdint.h>

//
// demodam
//
struct demodam
{
	demod_t demod;										// parent class
	
};
typedef struct demodam demodam_t;





/**
 *
 */
int demodam_init (demodam_t*, opool_t*);

/**
 *
 */
int demodam_destroy (demodam_t*);





/**
 *
 */
int demodam_feed (demodam_t*, uint32_t*, double*);

/**
 *
 */
int demodam_reset (demodam_t*);





/**
 *
 */
demodam_t* demodam_retain (demodam_t*);

/**
 *
 */
void demodam_release (demodam_t*);

#endif /* __DEMOD_AM_H__ */
