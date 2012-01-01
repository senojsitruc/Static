/*
 *  cpx2pwr.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "cpx2pwr.h"
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
cpx2pwr_init (cpx2pwr_t *cpx2pwr, double maxdb, double mindb, opool_t *pool)
{
	int error;
	
	if (unlikely(cpx2pwr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2pwr_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)cpx2pwr, "dsp-other-cpx2pwr", (cobject_destroy_func)cpx2pwr_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	cpx2pwr->maxdb = maxdb;
	cpx2pwr->mindb = mindb;
	cpx2pwr->ampmax = 32767.;
	cpx2pwr->dbcomp = 0.;
	cpx2pwr->dbgain = -10. / (maxdb - mindb);
	cpx2pwr->dbmaxoffset = maxdb / 10.;
	
	cpx2pwr->dsp.__feed = (__dsp_feed_func)cpx2pwr_feed;
	cpx2pwr->dsp.__reset = (__dsp_reset_func)cpx2pwr_reset;
	
	return 0;
}

/**
 *
 *
 */
int
cpx2pwr_destroy (cpx2pwr_t *cpx2pwr)
{
	int error;
	
	if (unlikely(cpx2pwr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2pwr_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)cpx2pwr))))
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
cpx2pwr_feed (cpx2pwr_t *cpx2pwr, uint32_t *size, double *data)
{
	uint32_t i, j, count;
	double phase_i, phase_q, dbgain, dbmaxoffset, kb, kc, dbrange;
	
	if (unlikely(cpx2pwr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2pwr_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = (*size / sizeof(double)) / 2;
	dbgain = cpx2pwr->dbgain;
	dbmaxoffset = cpx2pwr->dbmaxoffset;
	dbrange = cpx2pwr->maxdb - cpx2pwr->mindb;
	kb = cpx2pwr->dbcomp - 20 * log10(count * cpx2pwr->ampmax / 2.0);
	kc = pow(10., (cpx2pwr->mindb - kb) / 10.);
	kb = kb / 10.;
	
	for (i=0, j=0; i < count; ++i, j+=2) {
		phase_i = *(data+j+0);
		phase_q = *(data+j+1);
		
		double x0r = (phase_i*phase_i) + (phase_q*phase_q);
		double x0i = log10( x0r + kc ) + kb;
		double x0x = dbgain * (x0i - dbmaxoffset);
		
		data[i] = dbrange - (dbrange * x0x);
	}
	
	// the amount of meaningful data in "data" is now half what it was previously
	*size = *size / 2;
	
	return 0;
}

/**
 *
 *
 */
int
cpx2pwr_reset (cpx2pwr_t *cpx2pwr)
{
	if (unlikely(cpx2pwr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2pwr_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline cpx2pwr_t*
cpx2pwr_retain (cpx2pwr_t *cpx2pwr)
{
	if (unlikely(cpx2pwr == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null cpx2pwr_t");
	
	return (cpx2pwr_t*)dsp_retain((dsp_t*)cpx2pwr);
}

/**
 *
 *
 */
inline void
cpx2pwr_release (cpx2pwr_t *cpx2pwr)
{
	if (unlikely(cpx2pwr == NULL))
		LOG_ERROR_AND_RETURN(, "null cpx2pwr_t");
	
	dsp_release((dsp_t*)cpx2pwr);
}

