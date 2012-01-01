/*
 *  average.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "average.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
average_init (average_t *average, uint32_t data_size, uint32_t avg_size, opool_t *pool)
{
	int error;
	
	if (unlikely(average == NULL))
		LOG_ERROR_AND_RETURN(-1, "null average_t");
	
	if (unlikely(data_size == 0 || data_size > 10000))
		LOG_ERROR_AND_RETURN(-2, "invalid data_size (%u)", data_size);
	
	if (unlikely(avg_size == 0 || avg_size > 1000))
		LOG_ERROR_AND_RETURN(-3, "invalid avg_size (%u)", avg_size);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)average, "dsp-other-average", (cobject_destroy_func)average_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	if (unlikely(NULL == (average->sum = (double*)malloc(sizeof(double) * data_size))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc(sum), %s", strerror(errno));
	
	if (unlikely(NULL == (average->avg = (double*)malloc(sizeof(double) * data_size))))
		LOG_ERROR_AND_RETURN(-103, "failed to malloc(avg), %s", strerror(errno));
	
	memset(average->sum, 0, sizeof(double) * data_size);
	memset(average->avg, 0, sizeof(double) * data_size);
	
	average->data_size = data_size;
	average->avg_size = avg_size;
	average->avg_count = 0;
	
	average->dsp.__feed = (__dsp_feed_func)average_feed;
	average->dsp.__reset = (__dsp_reset_func)average_reset;
	
	return 0;
}

/**
 *
 *
 */
int
average_destroy (average_t *average)
{
	int error;
	double *sum, *avg;
	
	if (unlikely(average == NULL))
		LOG_ERROR_AND_RETURN(-1, "null average_t");
	
	sum = average->sum;
	avg = average->avg;
	
	if (sum != NULL && ATOMIC_CASPTR_BARRIER(sum, NULL, (void**)&average->sum))
		free(sum);
	
	if (avg != NULL && ATOMIC_CASPTR_BARRIER(avg, NULL, (void**)&average->avg))
		free(avg);
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)average))))
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
average_feed (average_t *average, uint32_t *size, double *data)
{
	uint32_t len, cnt;
	double *avg, *sum, *val, avg1, sum1, val1;
	
	if (unlikely(average == NULL))
		LOG_ERROR_AND_RETURN(-1, "null average_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	if (unlikely(*size != average->data_size * sizeof(double)))
		LOG_ERROR_AND_RETURN(-4, "mismatched sizes (size=%u, average->data_size=%lu)", *size, (average->data_size*sizeof(double)));
	
	avg = average->avg;
	sum = average->sum;
	val = data;
	len = average->data_size;
	cnt = average->avg_count;
	
	// the user has defined the "weight" of the average-ness which is a number of samples. if we don't
	// get have that number of samples, we just add to the sum directly and divide by the number of
	// samples that we do have so far.
	if (cnt < average->avg_size) {
		average->avg_count = ++cnt;
		
		for (uint32_t i = 0; i < len; ++i) {
//		avg1 = *avg;
			sum1 = *sum;
			val1 = *val;
			
			sum1 = sum1 + val1;
			avg1 = sum1 / cnt;
			
			*avg = avg1;
			*sum = sum1;
			*val = avg1;
			
			avg++;
			sum++;
			val++;
		}
		
		// we haven't yet filled our average-ing size, so don't let any other dsp processes work on our
		// data yet.
		*size = 0;
	}
	
	// otherwise, we do have the desired number of samples, so we decrease the sum by the average and
	// increase the sum by the new value; then divide by the user's defined sample size.
	else {
		for (uint32_t i = 0; i < len; ++i) {
			avg1 = *avg;
			sum1 = *sum;
			val1 = *val;
			
			sum1 = (sum1 - avg1) + val1;
			avg1 = sum1 / cnt;
			
			*avg = avg1;
			*sum = sum1;
			*val = avg1;
			
			avg++;
			sum++;
			val++;
		}
	}
	
	return 0;
}

/**
 *
 *
 */
int
average_reset (average_t *average)
{
	if (unlikely(average == NULL))
		LOG_ERROR_AND_RETURN(-1, "null average_t");
	
	memset(average->sum, 0, sizeof(double) * average->data_size);
	memset(average->avg, 0, sizeof(double) * average->data_size);
	
	average->avg_count = 0;
	
	return 0;
}	





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline average_t*
average_retain (average_t *average)
{
	if (unlikely(average == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null average_t");
	
	return (average_t*)dsp_retain((dsp_t*)average);
}

/**
 *
 *
 */
inline void
average_release (average_t *average)
{
	if (unlikely(average == NULL))
		LOG_ERROR_AND_RETURN(, "null average_t");
	
	dsp_release((dsp_t*)average);
}

