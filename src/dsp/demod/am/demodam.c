/*
 *  demodam.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.31.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "demodam.h"
#include "../../../misc/logger.h"
#include <math.h>





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
demodam_init (demodam_t *demodam, opool_t *pool)
{
	int error;
	
	if (unlikely(demodam == NULL))
		LOG_ERROR_AND_RETURN(-1, "null demodam_t");
	
	if (unlikely(0 != (error = demod_init((demod_t*)demodam, "AM Demod", (cobject_destroy_func)demodam_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to demod_init, %d", error);
	
	demodam->demod.__feed = (__dsp_feed_func)demodam_feed;
	demodam->demod.__reset = (__dsp_reset_func)demodam_reset;
	
	return 0;
}

/**
 *
 *
 */
int
demodam_destroy (demodam_t *demodam)
{
	int error;
	
	if (unlikely(demodam == NULL))
		LOG_ERROR_AND_RETURN(-1, "null demodam_t");
	
	if (unlikely(0 != (error = demod_destroy((demod_t*)demodam))))
		LOG_ERROR_AND_RETURN(-101, "failed to demod_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
demodam_feed (demodam_t *demodam, uint32_t *size, double *data)
{
	uint32_t i, count;
	double /*phase_i, phase_q, power,*/ *dataptr;
	
	if (unlikely(demodam == NULL))
		LOG_ERROR_AND_RETURN(-1, "null demodam_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(double);
	dataptr = data;
	
	for (i = 0; i < count; i+=2) {
//	phase_i = *(dataptr+0);
//	phase_q = *(dataptr+1);
//	power = sqrt((phase_i*phase_i) + (phase_q*phase_q));
		
		/*
		if (i == 200)
			LOG3("[%04d] phase_i=%lf, phase_q=%lf, power=%lf", i, phase_i, phase_q, power);
		*/
		
		dataptr += 2;
	}
	
	return 0;
}

/**
 *
 *
 */
int
demodam_reset (demodam_t *demodam)
{
	if (unlikely(demodam == NULL))
		LOG_ERROR_AND_RETURN(-1, "null demodam_t");
	
	return 0;
}





#pragma mark -
#pragma mark structors

/**
 *
 */
inline demodam_t*
demodam_retain (demodam_t *demodam)
{
	if (unlikely(demodam == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null demodam_t");
	
	return (demodam_t*)demod_retain((demod_t*)demodam);
}

/**
 *
 */
inline void
demodam_release (demodam_t *demodam)
{
	if (unlikely(demodam == NULL))
		LOG_ERROR_AND_RETURN(, "null demodam_t");
	
	demod_release((demod_t*)demodam);
}
