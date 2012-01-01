/*
 *  short2double.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  Converts a list of shorts to doubles. We don't know how many shorts are in the data, so we just
 *  assume that it has as many as will fit. A short is one-quarter the size of a double, so we 
 *  write from the end of that short list to the end of the data array and work our way back towards
 *  the beginning.
 *
 */

#ifndef __SHORT2DOUBLE_H__
#define __SHORT2DOUBLE_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// short2double
//
struct short2double
{
	dsp_t dsp;												// parent class
};
typedef struct short2double short2double_t;





/**
 * short2double, pool
 */
int short2double_init (short2double_t*, opool_t*);

/**
 *
 */
int short2double_destroy (short2double_t*);





/**
 *
 */
int short2double_feed (short2double_t*, uint32_t*, int16_t*);

/**
 *
 */
int short2double_reset (short2double_t*);





/**
 *
 */
short2double_t* short2double_retain (short2double_t*);

/**
 *
 */
void short2double_release (short2double_t*);

#endif /* __SHORT2DOUBLE_H__ */
