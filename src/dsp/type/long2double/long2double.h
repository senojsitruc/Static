/*
 *  long2double.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  Converts a list of longs to doubles. We don't know how many longs are in the data, so we just
 *  assume that it has as many as will fit. A long is assumed to be the size of a double.
 *
 */

#ifndef __LONG2DOUBLE_H__
#define __LONG2DOUBLE_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// long2double
//
struct long2double
{
	dsp_t dsp;												// parent class
};
typedef struct long2double long2double_t;





/**
 * long2double, pool
 */
int long2double_init (long2double_t*, opool_t*);

/**
 *
 */
int long2double_destroy (long2double_t*);





/**
 *
 */
int long2double_feed (long2double_t*, uint32_t*, double*);

/**
 *
 */
int long2double_reset (long2double_t*);





/**
 *
 */
long2double_t* long2double_retain (long2double_t*);

/**
 *
 */
void long2double_release (long2double_t*);

#endif /* __LONG2DOUBLE_H__ */
