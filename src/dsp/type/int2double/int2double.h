/*
 *  int2double.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  Converts a list of ints to doubles. We don't know how many ints are in the data, so we just
 *  assume that it has as many as will fit. An int is one-half the size of a double, so we cop-and-
 *  convert from the end of the int list to the end of the data array and work our way back towards
 *  the beginning.
 *
 */

#ifndef __INT2DOUBLE_H__
#define __INT2DOUBLE_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// int2double
//
struct int2double
{
	dsp_t dsp;												// parent class
};
typedef struct int2double int2double_t;





/**
 * int2double, pool
 */
int int2double_init (int2double_t*, opool_t*);

/**
 *
 */
int int2double_destroy (int2double_t*);





/**
 *
 */
int int2double_feed (int2double_t*, uint32_t*, double*);

/**
 *
 */
int int2double_reset (int2double_t*);





/**
 *
 */
int2double_t* int2double_retain (int2double_t*);

/**
 *
 */
void int2double_release (int2double_t*);

#endif /* __INT2DOUBLE_H__ */
