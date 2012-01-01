/*
 *  cpx2pwr.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------*
 *
 *  Complex-to-Power. 28 * log10( i^2 + q^2 )
 *
 */

#ifndef __CPX2PWR_H__
#define __CPX2PWR_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// cpx2pwr
//
struct cpx2pwr
{
	dsp_t dsp;												// parent class
	
	double maxdb;											// max db = 0.
	double mindb;											// min db = -140.
	
	double ampmax;										// amp max = 32767.
	double dbcomp;										// db compensation = 0.0
	double dbgain;										// db gain = -10. / (maxdb - mindb)
	double dbmaxoffset;								// db max offset = maxdb / 10.
};
typedef struct cpx2pwr cpx2pwr_t;





/**
 * cpx2pwr, maxdb, mindb, pool
 */
int cpx2pwr_init (cpx2pwr_t*, double, double, opool_t*);

/**
 *
 */
int cpx2pwr_destroy (cpx2pwr_t*);





/**
 *
 */
int cpx2pwr_feed (cpx2pwr_t*, uint32_t*, double*);

/**
 *
 */
int cpx2pwr_reset (cpx2pwr_t*);





/**
 *
 */
cpx2pwr_t* cpx2pwr_retain (cpx2pwr_t*);

/**
 *
 */
void cpx2pwr_release (cpx2pwr_t*);

#endif /* __CPX2PWR_H__ */
