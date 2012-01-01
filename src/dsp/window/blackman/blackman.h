/*
 *  blackman.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  http://en.wikipedia.org/wiki/Window_function#Blackman_windows
 *  https://ccrma.stanford.edu/~jos/st/Use_Blackman_Window.html
 *
 */

#ifndef __WINDOW_BLACKMAN_H__
#define __WINDOW_BLACKMAN_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// blackman
//
struct blackman
{
	dsp_t dsp;												// parent class
	
	uint32_t size;										// window size
	double offset;										// adjustment
	double *window;										// window
};
typedef struct blackman blackman_t;





/**
 * blackman, window size, adjustment, pool
 */
int blackman_init (blackman_t*, uint32_t, double, opool_t*);

/**
 *
 */
int blackman_destroy (blackman_t*);





/**
 *
 */
int blackman_feed (blackman_t*, uint32_t*, double*);

/**
 *
 */
int blackman_reset (blackman_t*);





/**
 *
 */
blackman_t* blackman_retain (blackman_t*);

/**
 *
 */
void blackman_release (blackman_t*);

#endif /* __WINDOW_BLACKMAN_H__ */
