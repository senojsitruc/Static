/*
 *  invert.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.07.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  Invert the data - that is, the values on the far-left of the array become the values on the far-
 *  right of the array. Invert assumes complex values.
 *
 *  TODO: init() should take an argument specifying REAL or COMPLEX.
 *
 */

#ifndef __INVERT_H__
#define __INVERT_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// invert_type
//
typedef enum
{
	INVERT_COMPLEX = (1<<0),
	INVERT_REAL = (1<<1)
} invert_type;

//
// invert
//
struct invert
{
	dsp_t dsp;												// parent class
	invert_type type;									// real, complex
};
typedef struct invert invert_t;





/**
 * invert, pool
 */
int invert_init (invert_t*, invert_type, opool_t*);

/**
 *
 */
int invert_destroy (invert_t*);





/**
 *
 */
int invert_feed (invert_t*, uint32_t*, double*);

/**
 *
 */
int invert_reset (invert_t*);





/**
 *
 */
invert_t* invert_retain (invert_t*);

/**
 *
 */
void invert_release (invert_t*);

#endif /* __INVERT_H__ */
