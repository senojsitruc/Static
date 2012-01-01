/*
 *  invert.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.07.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "invert.h"
#include "../../../device/device.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
invert_init (invert_t *invert, invert_type type, opool_t *pool)
{
	int error;
	
	if (unlikely(invert == NULL))
		LOG_ERROR_AND_RETURN(-1, "null invert_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)invert, "dsp-other-invert", (cobject_destroy_func)invert_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	invert->type = type;
	
	invert->dsp.__feed = (__dsp_feed_func)invert_feed;
	invert->dsp.__reset = (__dsp_reset_func)invert_reset;
	
	return 0;
}

/**
 *
 *
 */
int
invert_destroy (invert_t *invert)
{
	int error;
	
	if (unlikely(invert == NULL))
		LOG_ERROR_AND_RETURN(-1, "null invert_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)invert))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
invert_feed (invert_t *invert, uint32_t *size, double *data)
{
	uint32_t i, count;
	
	if (unlikely(invert == NULL))
		LOG_ERROR_AND_RETURN(-1, "null invert_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(double);
	
	// real
	if (INVERT_REAL == invert->type) {
		double power1, power2;
		double *data_beg, *data_end;
		
		data_beg = data;
		data_end = data + count - 1;
		
		for (i = 0; i < count; i+=2) {
			power1 = *data_beg;
			power2 = *data_end;
			
			*data_beg = power2;
			*data_end = power1;
			
			data_beg++;
			data_end--;
		}
	}
	
	// complex
	else if (INVERT_COMPLEX == invert->type) {
		double phase_i1, phase_q1, phase_i2, phase_q2;
		double *data_beg1, *data_beg2, *data_end1, *data_end2;
		
		data_beg1 = data;
		data_beg2 = data + 1;
		data_end1 = data + count - 2;
		data_end2 = data + count - 1;
		
		for (i = 0; i < count; i+=4) {
			phase_i1 = *data_beg1;
			phase_q1 = *data_beg2;
			phase_i2 = *data_end1;
			phase_q2 = *data_end2;
			
			*data_beg1 = phase_i2;
			*data_beg2 = phase_q2;
			*data_end1 = phase_i1;
			*data_end2 = phase_q1;
			
			data_beg1 += 2;
			data_beg2 += 2;
			data_end1 -= 2;
			data_end2 -= 2;
		}
	}
	
	else
		LOG_ERROR_AND_RETURN(-1, "unsupported invert type, 0x%04X", invert->type);
	
	return 0;
}

/**
 *
 *
 */
int
invert_reset (invert_t *invert)
{
	if (unlikely(invert == NULL))
		LOG_ERROR_AND_RETURN(-1, "null invert_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline invert_t*
invert_retain (invert_t *invert)
{
	if (unlikely(invert == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null invert_t");
	
	return (invert_t*)dsp_retain((dsp_t*)invert);
}

/**
 *
 *
 */
inline void
invert_release (invert_t *invert)
{
	if (unlikely(invert == NULL))
		LOG_ERROR_AND_RETURN(, "null invert_t");
	
	dsp_release((dsp_t*)invert);
}
