/*
 *  double2sint16.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.29.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __DSP_DOUBLE_TO_SINT16_H__
#define __DSP_DOUBLE_TO_SINT16_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"
#include <stdint.h>

//
// double2sint16
//
struct double2sint16
{
	dsp_t dsp;                           // parent class
};
typedef struct double2sint16 double2sint16_t;





/**
 * double2sint16, pool
 */
int double2sint16_init (double2sint16_t*, opool_t*);

/**
 *
 */
int double2sint16_destroy (double2sint16_t*);





/**
 *
 */
int double2sint16_feed (double2sint16_t*, uint32_t*, double*);

/**
 *
 */
int double2sint16_reset (double2sint16_t*);





/**
 *
 */
double2sint16_t* double2sint16_retain (double2sint16_t*);

/**
 *
 */
void double2sint16_release (double2sint16_t*);

#endif /* __DSP_DOUBLE_TO_SINT16_H__ */
