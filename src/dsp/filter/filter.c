/*
 *  filter.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "filter.h"
#include "../../misc/logger.h"
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
filter_init (filter_t *filter, filter_method method, uint32_t output_l, char *name, cobject_destroy_func destroy, opool_t *pool)
{
	int error;
	
	if (unlikely(filter == NULL))
		LOG_ERROR_AND_RETURN(-1, "null filter_t");
	
	if (unlikely(output_l == 0))
		LOG_ERROR_AND_RETURN(-2, "invalid output_l (%u)", output_l);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)filter, name, destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	if (unlikely(NULL == (filter->output = (double*)malloc(sizeof(double) * output_l))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc(%lu), %s", (sizeof(double)*output_l), strerror(errno));
	
	memset(filter->output, 0, sizeof(double) * output_l);
	
	filter->method = method;
	filter->output_l = output_l;
	
	/*
	filter->dsp.__feed = (__dsp_feed_func)filter_feed;
	filter->dsp.__reset = (__dsp_reset_func)filter_reset;
	*/
	
	return 0;
}

/**
 *
 *
 */
int
filter_destroy (filter_t *filter)
{
	int error;
	
	if (unlikely(filter == NULL))
		LOG_ERROR_AND_RETURN(-1, "null filter_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)filter))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 * Recursion - http://www.dspguide.com/ch26/6.htm
 *
 */
int
filter_recurse (filter_t *filter, uint32_t poles, uint32_t input_l, double *input, uint32_t coef_a_l, double *coef_a, uint32_t coef_b_l, double *coef_b)
{
	uint32_t i, j;
	
	if (unlikely(filter == NULL))
		LOG_ERROR_AND_RETURN(-1, "null filter_t");
	
	if (unlikely(poles == 0))
		LOG_ERROR_AND_RETURN(-2, "invalid number of poles (%u)", poles);
	
	if (unlikely(input_l != filter->output_l))
		LOG_ERROR_AND_RETURN(-3, "input signal length (%u) != output signal buffer (%u)", input_l, filter->output_l);
	
	if (unlikely(input == NULL))
		LOG_ERROR_AND_RETURN(-4, "null input");
	
	if (unlikely(coef_a == NULL))
		LOG_ERROR_AND_RETURN(-5, "null coef_a");
	
	if (unlikely(coef_a_l == 0 || coef_a_l > 1000))
		LOG_ERROR_AND_RETURN(-6, "invalid coef_a_l (%u)", coef_a_l);
	
	if (unlikely(coef_b == NULL))
		LOG_ERROR_AND_RETURN(-7, "null coef_b");
	
	if (unlikely(coef_b_l == 0 || coef_b_l > 1000))
		LOG_ERROR_AND_RETURN(-8, "invalid coef_b_l (%u)", coef_b_l);
	
	/*
	for (i = (poles*2), k = poles; i < input_l; i+=2, k+=1) {
		filter->output[k] = coef_a[0] * input[i];
		
		for (j = 1; j <= poles; ++j)
			filter->output[k] = filter->output[k] * coef_a[j] * input[i-(j*2)] + coef_b[j] * filter->output[k-j];
	}
	*/
	
//memset(filter->output, 0, sizeof(double) * filter->output_l);
	
	/*
	for (i = poles; i < input_l; ++i) {
//	filter->output[i] = coef_a[0] * input[i];
		
		for (j = 1; j <= poles; ++j)
			filter->output[i] = filter->output[i] * coef_a[j] * input[i-j] + coef_b[j] * filter->output[i-j];
	}
	*/
	
	double *rel = malloc(sizeof(double) * input_l / 2);
	double *img = malloc(sizeof(double) * input_l / 2);
	
	for (i = 0; i < input_l / 2; ++i) {
		rel[i] = input[i*2+0];
		img[i] = input[i*2+1];
	}
	
	for (i = poles; i < input_l / 2; ++i) {
		for (j = 1; j <= poles; ++j)
			rel[i] = rel[i] + coef_a[j] * img[i-j] + coef_b[j] * rel[i-j];
	}
	
	/*
	for (i = poles*2; i < input_l-1; i += 2) {
		for (j = 0; j < poles; ++j)
			input[i] = input[i] + coef_a[j] * input[(i+1)-(j*2)] + coef_b[j] * input[i-(j*2)];
	}
	
	// the real and imaginary values are interleaved. copy all of the real values to the bottom half
	// of the input array (which over-writes the imaginary values in the bottom half).
	for (i=1, j=2; j < input_l-1; i+=1, j+=2)
		input[i] = input[j];
	*/
	
	for (i=0, j=0; j < input_l; i+=1, j+=2) {
		input[j+0] = rel[i];
		input[j+1] = img[i];
	}
	
	/*
	for (i = 0; i < input_l / 2; ++i)
		input[i] = rel[i];
	*/
	
	free(rel);
	free(img);
	
	// complex input, real output: resize
	
	return 0;
}

/**
 * Convolution - http://www.dspguide.com/ch6.htm
 *
 */
int
filter_convolve (filter_t *filter, uint32_t input_l, double *input, uint32_t impulse_l, double *impulse)
{
	uint32_t i, j;
	
	if (unlikely(filter == NULL))
		LOG_ERROR_AND_RETURN(-1, "null filter_t");
	
	if (unlikely(input == NULL))
		LOG_ERROR_AND_RETURN(-2, "null input");
	
	if (unlikely(impulse == NULL))
		LOG_ERROR_AND_RETURN(-3, "null impulse");
	
	if (unlikely(input_l + impulse_l > filter->output_l))
		LOG_ERROR_AND_RETURN(-4, "input.len (%u) + impulse.len (%u) > output.len (%u)", input_l, impulse_l, filter->output_l);
	
	memset(filter->output, 0, sizeof(double) * filter->output_l);
	
	for (i = 0; i < input_l; ++i)
		for (j = 0; j < impulse_l; ++j)
			filter->output[i+j] = filter->output[i+j] + input[i] * impulse[j];
	
	return 0;
}

/**
 *
 *
 */
int
filter_reset (filter_t *filter)
{
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 */
filter_t*
filter_retain (filter_t *filter)
{
	if (unlikely(filter == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null filter_t");
	
	return (filter_t*)dsp_retain((dsp_t*)filter);
}

/**
 *
 *
 */
void
filter_release (filter_t *filter)
{
	if (unlikely(filter == NULL))
		LOG_ERROR_AND_RETURN(, "null filter_t");
	
	dsp_release((dsp_t*)filter);
}
