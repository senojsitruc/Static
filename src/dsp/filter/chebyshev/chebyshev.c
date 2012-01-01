/*
 *  chebyshev.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "chebyshev.h"
#include "../../dsp.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <math.h>
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
chebyshev_init (chebyshev_t *chebyshev, double cutoff, chebyshev_type type, double ripple, uint32_t poles, uint32_t signal_l, opool_t *pool)
{
	int error;
	
	if (unlikely(chebyshev == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chebyshev_t");
	
	if (unlikely(cutoff < 0.0 || cutoff > 0.5))
		LOG_ERROR_AND_RETURN(-2, "invalid frequency cutoff (%lf)", cutoff);
	
	if (unlikely(ripple < 0. || ripple > 29.))
		LOG_ERROR_AND_RETURN(-3, "invalid filter frequency ripple response (%lf)", ripple);
	
	if (unlikely(poles < 2 || poles > 20 || 0 != poles % 2))
		LOG_ERROR_AND_RETURN(-4, "invalid number of poles (%u)", poles);
	
	if (unlikely(0 != (error = filter_init((filter_t*)chebyshev, FILTER_METHOD_RECURSE, signal_l, "dsp-other-chebyshev", (cobject_destroy_func)chebyshev_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	chebyshev->cutoff = cutoff;
	chebyshev->ripple = ripple;
	chebyshev->poles = poles;
	chebyshev->type = type;
	
	chebyshev->filter.dsp.__feed = (__dsp_feed_func)chebyshev_feed;
	chebyshev->filter.dsp.__reset = (__dsp_reset_func)chebyshev_reset;
	
	// calculates the filter coefficients
	chebyshev_reset(chebyshev);
	
	return 0;
}

/**
 *
 *
 */
int
chebyshev_destroy (chebyshev_t *chebyshev)
{
	int error;
	
	if (unlikely(chebyshev == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chebyshev_t");
	
	if (unlikely(0 != (error = filter_destroy((filter_t*)chebyshev))))
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
chebyshev_feed (chebyshev_t *chebyshev, uint32_t *size, double *data)
{
	int error;
	uint32_t _size;
	size_t count;
	
	if (unlikely(chebyshev == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chebyshev_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(*size == 0))
		LOG_ERROR_AND_RETURN(-3, "invalid size (%u)", *size);
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-4, "null data");
	
	_size = *size;
	count = _size / sizeof(double);
	
	if (unlikely(0 != (error = filter_recurse((filter_t*)chebyshev, (uint32_t)chebyshev->poles, (uint32_t)count, data, 23, chebyshev->coef_a, 23, chebyshev->coef_b))))
		LOG_ERROR_AND_RETURN(-101, "failed to filter_recurse(), %d", error);
	
//memcpy(data, chebyshev->filter.output, sizeof(double) * MIN(chebyshev->filter.output_l, count));
	
//for (size_t i=0, j=0; i < count; i+=2, j+=1)
//	*(data+i) = *(chebyshev->filter.output+j);
	
//*size = (uint32_t)(sizeof(double) * MIN(chebyshev->filter.output_l, count) / 2);
	
	return 0;
}

/**
 *
 *
 */
int
chebyshev_reset (chebyshev_t *chebyshev)
{
	if (unlikely(chebyshev == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chebyshev_t");
	
	LOG3("cutoff=%lf, type=%s, ripple=%lf, poles=%lf",
			 chebyshev->cutoff, (chebyshev->type==CHEBYSHEV_LOW_PASS?"low-pass":"high-pass"),
			 chebyshev->ripple, chebyshev->poles);
	
	int i, p;
	double gain, sa=0., sb=0.;
	double a_tmp[23]={0}, b_tmp[23]={0};
	double a0, a1, a2, b1, b2;
	
	// zero out the filter coefficients
	memset(chebyshev->coef_a, 0, sizeof(chebyshev->coef_a));
	memset(chebyshev->coef_b, 0, sizeof(chebyshev->coef_b));
	
	chebyshev->coef_a[2] = 1.;
	chebyshev->coef_b[2] = 1.;
	
	for (p = 1; p <= chebyshev->poles/2; ++p) {
		double rp=0., ip=0., es=0., vx=0., kx=0., t=0., w=0., m=0., d=0., k=0.;
		double x0=0., x1=0., x2=0., y1=0., y2=0;
		
		// calculate the pole location on the unit circle
		rp = - cos(M_PI / (chebyshev->poles*2) + (p-1) * (M_PI/chebyshev->poles));
		ip = sin(M_PI / (chebyshev->poles*2) + (p-1) * (M_PI/chebyshev->poles));
		
		// warp from a circle to an ellipse
		if (chebyshev->ripple != 0.) {
			es = sqrt(pow(100./(100.-chebyshev->ripple),2.) - 1);
			vx = (1. / chebyshev->poles) * log((1./es) + sqrt(1./(es*es)+1));
			kx = (1. / chebyshev->poles) * log((1./es) + sqrt(1./(es*es)-1));
			kx = (exp(kx) + exp(-kx)) / 2.;
			rp = rp * ((exp(vx) - exp(-vx)) / 2) / kx;
			ip = ip * ((exp(vx) + exp(-vx)) / 2) / kx;
		}
		
		LOG3("rp=%lf, ip=%lf, es=%lf, vx=%lf, kx=%lf", rp, ip, es, vx, kx);
		
		// s-domain to z-domain conversion
		{
			t = 2. * tan(.5);
			w = 2. * M_PI * chebyshev->cutoff;
			m = (rp*rp) + (ip*ip);
			d = 4. - (4. * rp * t) + (m * (t*t));
			x0 = (t*t) / d;
			x1 = (2. * (t*t)) / d;
			x2 = (t*t) / d;
			y1 = (8. - (2.*m*(t*t))) / d;
			y2 = (-4. - (4.*rp*t) - (m*(t*t))) / d;
		}
		
		LOG3("t=%lf, w=%lf, m=%lf, d=%lf", t, w, m, d);
		LOG3("x0=%lf, x1=%lf, x2=%lf, y1=%lf, y2=%lf", x0, x1, x2, y1, y2);
		
		// low-pass to low-pass or low-pass to high-pass transform
		{
			if (CHEBYSHEV_LOW_PASS == chebyshev->type)
				k = sin(.5 - (w/2.)) / sin(.5 + (w/2.));
			else if (CHEBYSHEV_HIGH_PASS == chebyshev->type)
				k = -cos((w/2.) + .5) / cos((w/2.) - .5);
			
			d  = 1. + (y1*k) - (y2*(k*k));
			a0 = (x0 - (x1*k) + (x2*(k*k))) / d;
			a1 = ((-2.*x0*k) + x1 + (x1*(k*k)) - (2*x2*k)) / d;
			a2 = ((x0*(k*k)) - (x1*k) + x2) / d;
			b1 = ((2.*k) + y1 + (y1*(k*k)) - (2.*y2*k)) / d;
			b2 = (-(k*k) - (y1*k) + y2) / d;
			
			if (CHEBYSHEV_HIGH_PASS == chebyshev->type) {
				a1 = -a1;
				b1 = -b1;
			}
		}
		
		LOG3("a0=%lf, a1=%lf, a2=%lf, b1=%lf, b2=%lf", a0, a1, a2, b1, b2);
		
		memcpy(a_tmp, chebyshev->coef_a, sizeof(a_tmp));
		memcpy(b_tmp, chebyshev->coef_b, sizeof(b_tmp));
		
		for (i = 2; i <= 22; ++i) {
			chebyshev->coef_a[i] = (a0 * a_tmp[i]) + (a1 * a_tmp[i-1]) + (a2 * a_tmp[i-2]);
			chebyshev->coef_b[i] = b_tmp[i] - (b1 * b_tmp[i-1]) - (b2 * b_tmp[i-2]);
		}
	}
	
	chebyshev->coef_b[2] = 0;
	
	for (i = 0; i <= 20; ++i) {
		chebyshev->coef_a[i] = chebyshev->coef_a[i+2];
		chebyshev->coef_b[i] = -chebyshev->coef_b[i+2];
	}
	
	if (CHEBYSHEV_LOW_PASS == chebyshev->type) {
		for (i = 0; i <= 20; ++i) {
			sa = sa + chebyshev->coef_a[i];
			sb = sb + chebyshev->coef_b[i];
		}
	}
	else if (CHEBYSHEV_HIGH_PASS == chebyshev->type) {
		for (i = 0; i <= 20; ++i) {
			sa = sa + chebyshev->coef_a[i] * pow(-1.,i);
			sb = sb + chebyshev->coef_b[i] * pow(-1.,i);
		}
	}
	
	gain = sa / (1 - sb);
	
	for (i = 0; i <= 20; ++i)
		chebyshev->coef_a[i] = chebyshev->coef_a[i] / gain;
	
	for (i = 0; i <= 20; ++i)
		LOG3("coef_a[%02d] = %f", i, chebyshev->coef_a[i]);
	
	for (i = 0; i <= 20; ++i)
		LOG3("coef_b[%02d] = %f", i, chebyshev->coef_b[i]);
	
	return 0;
}	





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline chebyshev_t*
chebyshev_retain (chebyshev_t *chebyshev)
{
	if (unlikely(chebyshev == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null chebyshev_t");
	
	return (chebyshev_t*)filter_retain((filter_t*)chebyshev);
}

/**
 *
 *
 */
inline void
chebyshev_release (chebyshev_t *chebyshev)
{
	if (unlikely(chebyshev == NULL))
		LOG_ERROR_AND_RETURN(, "null chebyshev_t");
	
	filter_release((filter_t*)chebyshev);
}

