/*
 *  history.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.24.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "history.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRAPH_HISTORY_MAX_X 2048
#define GRAPH_HISTORY_MAX_Y 1000





#pragma mark -
#pragma mark private methods

static int __graphhistory_draw (graphhistory_t*, void*);
static int __graphhistory_feed (graphhistory_t*, uint32_t, double*);
static int __graphhistory_ready (graphhistory_t*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
graphhistory_init (graphhistory_t *gh, char *name, uint32_t size, opool_t *pool)
{
	int error;
	
	if (unlikely(gh == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphhistory_t");
	
	// initialize the parent class
	if (unlikely(0 != (error = graph_init((graph_t*)gh, GRAPH_2D, name, size, (cobject_destroy_func)graphhistory_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to graph_init, %d", name, error);
	
	// configuration
	gh->starttime = 0;
	gh->position = 0;
	gh->drawposition = 0;
	gh->valhigh = 0.;
	gh->vallow = 0.;
	
	// the function pointers are used by the parent class to tell us when we need to draw, process
	// new data or ready the graph (ie, allocate memory).
	gh->graph.__draw_fp = (__graph_draw_fp_func)__graphhistory_draw;
	gh->graph.__feed_fp = (__graph_feed_fp_func)__graphhistory_feed;
	gh->graph.__ready_fp = (__graph_ready_fp_func)__graphhistory_ready;
	
	return 0;
}

/**
 *
 *
 */
int
graphhistory_destroy (graphhistory_t *gh)
{
	int error;
	
	// TODO: free
	
	gh->graph.data = NULL;
	
	if (unlikely(0 != (error = graph_destroy((graph_t*)gh))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to graph_destroy, %d", gh->graph.name, error);
	
	
	
	
	return 0;
}

/**
 *
 *
 */
static int
__graphhistory_ready (graphhistory_t *gh)
{
	uint32_t width, height;
	
	if (unlikely(gh == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphhistory_t");
	
	// the width and height of the graph (in pixels)
	width = gh->graph.axis_x.length;
	height = gh->graph.axis_y.length;
	
	// limit the graph width to our maximum supported width
	if (unlikely(width > GRAPH_HISTORY_MAX_X))
		LOG_ERROR_AND_RETURN(-101, "[%s] width (%d) exceeds max (%d)", gh->graph.name, width, GRAPH_HISTORY_MAX_X);
	
	// limit the graph height to our maximum supported height
	if (unlikely(height > GRAPH_HISTORY_MAX_Y))
		LOG_ERROR_AND_RETURN(-102, "[%s] height (%d) exceeds max (%d)", gh->graph.name, height, GRAPH_HISTORY_MAX_Y);
	
	// configuration
	gh->drawrows = height * 2;
	gh->drawrow = gh->drawrows - height;
	gh->data.size1 = width * height * sizeof(double);
//gh->data.size2 = width * height * sizeof(uint32_t);
	gh->data.size2 = (uint32_t)(width * gh->drawrows * sizeof(uint32_t));
	gh->data.count = width * height;
	gh->colorsteps = 1280;
	
	// allocate the memory used to hold the graph data
	if (unlikely(NULL == (gh->data.data1 = (double*)malloc(gh->data.size1))))
		LOG_ERROR_AND_RETURN(-103, "[%s] failed to malloc(%d), %s", gh->graph.name, gh->data.size1, strerror(errno));
	
	// allocate the memory used to hold the graph data color
//if (unlikely(NULL == (gh->data.data2 = (uint32_t*)malloc(gh->data.size2))))
//	LOG_ERROR_AND_RETURN(-103, "[%s] failed to malloc(%d), %s", gh->graph.name, gh->data.size2, strerror(errno));
	if (unlikely(NULL == (gh->data.data2 = (uint32_t*)malloc(gh->data.size2))))
		LOG_ERROR_AND_RETURN(-103, "[%s] failed to malloc(%d), %s", gh->graph.name, gh->data.size2, strerror(errno));
	
	// allocate the memory used to hold the color spectrum
	if (unlikely(NULL == (gh->colors = (uint32_t*)malloc(gh->colorsteps * sizeof(uint32_t)))))
		LOG_ERROR_AND_RETURN(-105, "[%s] failed to malloc(%lu), %s", gh->graph.name, (gh->colorsteps*sizeof(uint32_t)), strerror(errno));
	
	// clear the memory used to hold the graph data, color spectrum
	memset(gh->data.data1, 0, gh->data.size1);
	memset(gh->data.data2, 0, gh->data.size2);
	memset(gh->colors, 0, gh->colorsteps * sizeof(uint32_t));
	
	// clears out and resets various things. take my word for it.
	graphhistory_reset(gh);
	
	// give the parent class a pointer to our main graph data structure
	gh->graph.data = &gh->data;
	
	// initialize the color spectrum
	{
		uint32_t r=0, g=0, b=0, a=255, *colors;
		int i, steps=256;
		
		colors = gh->colors;
		
		// b: 0 -> n
		for (i = 0; i < steps; ++i) {
			b++;
			*colors = (r<<24) | (g<<16) | (b<<8) | a;
			colors++;
		}
		
		// g: 0 -> n
		for (i = 0; i < steps; ++i) {
			g++;
			*colors = (r<<24) | (g<<16) | (b<<8) | a;
			colors++;
		}
		
		// b: n -> 0
		for (i = 0; i < steps; ++i) {
			b--;
			*colors = (r<<24) | (g<<16) | (b<<8) | a;
			colors++;
		}
		
		// r: 0 -> n
		for (i = 0; i < steps; ++i) {
			r++;
			*colors = (r<<24) | (g<<16) | (b<<8) | a;
			colors++;
		}
		
		// g: n -> 0
		for (i = 0; i < steps; ++i) {
			g--;
			*colors = (r<<24) | (g<<16) | (b<<8) | a;
			colors++;
		}
	}
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
graph_t*
graphhistory_graph (graphhistory_t *gh)
{
	return (graph_t*)gh;
}

/**
 *
 *
 */
int
graphhistory_reset (graphhistory_t *gh)
{
	uint32_t i, width, height;
	
	if (unlikely(gh == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphhistory_t");
	
	// the width and height of the graph (in pixels)
	width = gh->graph.axis_x.length;
	height = gh->graph.axis_y.length;
	
	// the graph has not been ready()'ed yet so don't reset anything
	if (width == 0 || height == 0 || gh->data.data1 == NULL || gh->data.data2 == NULL)
		return 0;
	
	// the calibration values that dictate the y-axis span
	gh->valhigh = 0.;
	gh->vallow = 0.;
	
	memset(gh->data.data1, 0, gh->data.size1);
	memset(gh->data.data2, 0, gh->data.size2);
	
	// initialize the "alpha" component of each color, so that the data-less rendering of this graph
	// is all black instead of transparent.
	for (i = 0; i < gh->drawrows * width; ++i)
		*(gh->data.data2+i) = 255;
	
	dspchain_reset(&gh->graph.dspchain);
	
	return 0;
}





#pragma mark -
#pragma mark draw & feed

/**
 *
 *
 */
static int
__graphhistory_draw (graphhistory_t *gh, void *context)
{
	uint32_t width, height;
	graph_draw_line_fp_func line_fp = gh->graph.draw_line_fp;
	graph_draw_line_bp_func line_bp = gh->graph.draw_line_bp;
	graph_draw_hist_fp_func hist_fp = gh->graph.draw_hist_fp;
	graph_draw_hist_bp_func hist_bp = gh->graph.draw_hist_bp;
	
	if (unlikely(gh == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphhistory_t");
	
	width = gh->graph.axis_x.length;
	height = gh->graph.axis_y.length;
	
	// we get called before we've even configured the axis'es.
	if (height == 0)
		return 0;
	
	// draw the history graph image
	if (hist_fp != NULL)
		(*hist_fp)((graph_t*)gh, context, gh->data.data2+(gh->drawrow*width), width, height);
	else if (hist_bp != NULL)
		hist_bp((graph_t*)gh, context, gh->data.data2+(gh->drawrow*width), width, height);
	
	// draw the white border around the graph
	if (line_fp != NULL) {
		(*line_fp)((graph_t*)gh, context, 0., 0., 0., height-1, 1., 1., 1., 1.);
		(*line_fp)((graph_t*)gh, context, 0., height-1, width-1, height-1, 1., 1., 1., 1.);
		(*line_fp)((graph_t*)gh, context, width-1, height-1, width-1, 0., 1., 1., 1., 1.);
		(*line_fp)((graph_t*)gh, context, width-1, 0., 0., 0., 1., 1., 1., 1.);
	}
	else if (line_bp != NULL) {
		line_bp((graph_t*)gh, context, 0., 0., 0., height-1, 1., 1., 1., 1.);
		line_bp((graph_t*)gh, context, 0., height-1, width-1, height-1, 1., 1., 1., 1.);
		line_bp((graph_t*)gh, context, width-1, height-1, width-1, 0., 1., 1., 1., 1.);
		line_bp((graph_t*)gh, context, width-1, 0., 0., 0., 1., 1., 1., 1.);
	}
	
	return 0;
}

/**
 *
 *
 */
static int
__graphhistory_feed (graphhistory_t *gh, uint32_t size, double *data)
{
	if (unlikely(gh == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphhistory_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-2, "null data");
	
	// if this is our first data sample we need to set the start time
	if (gh->starttime == 0)
		gh->starttime = graph_milliseconds();
	
	uint64_t i, dif;
	double *rowptr1, val;
	uint32_t width = gh->graph.axis_x.length;
	uint32_t height = gh->graph.axis_y.length;
	uint32_t count = size / sizeof(double);
	uint32_t steps_i = gh->colorsteps;
	double slottime = ((gh->graph.axis_y.trim_high - gh->graph.axis_y.trim_low) / (double)height) * 1000.;
	double valhigh = gh->valhigh;
	double vallow = gh->vallow;
	double steps_d = gh->colorsteps;
	uint64_t row = (uint64_t)((double)(graph_milliseconds() - gh->starttime) / slottime);
	
	if (row != gh->position) {
		for (i = gh->position+1; i <= row; ++i)
			memset(gh->data.data1+((i%height)*width), 0, sizeof(double)*width);
		
		gh->position = row;
		gh->positioncnt = 0;
	}
	
	row %= height;
	rowptr1 = gh->data.data1 + (row * width);
	
	// if the data size exceeds the graph width, reduce the data size to the graph width
	if (count > gh->graph.axis_x.length)
		count = gh->graph.axis_x.length;
	
	// we're doing an average here. another option is doing max.
	for (i = 0; i < count; ++i) {
		val = *data;
		
		if (val > valhigh)
			valhigh = val;
		
		if (vallow == 0. || val < vallow)
			vallow = val;
		
		//if (val > *slotptr)
		//	*slotptr = val;
		
		if (gh->positioncnt == 0)
			*rowptr1 = val;
		else
			*rowptr1 = ((*rowptr1 * gh->positioncnt) + val) / (gh->positioncnt+1);
		
		rowptr1++;
		data++;
	}
	
	gh->positioncnt++;
	gh->valhigh = valhigh;
	gh->vallow = vallow;
	
	
	// imagine a draw buffer with 'n' rows for a graph with a height of 'm' where n >= m for the sake
	// of minimizing memcpy()'s since the graph needs to be contiguous. the current draw row, 'r', as
	// determined by the amount of time that has passed is then found by (n-m) - (r % (n-m)), which 
	// causes us to iterate over row n-m to zero continuously.
	row = (gh->drawrows - height - 1) - (gh->position % (gh->drawrows - height));
	dif = row <= gh->drawrow ? (gh->drawrow - row) : (gh->drawrows - height - 1 - row + gh->drawrow);
	
	// we have looped around from the top back to the bottom and we need to copy the graph data from
	// the top of the draw buffer back to the bottom of the draw buffer.
	if (row > gh->drawrow && dif >= 1) {
		memcpy(gh->data.data2+((gh->drawrows-height)*width), gh->data.data2, height*width*sizeof(uint32_t));
		gh->drawrow = (gh->drawrows - height) + gh->drawrow;
	}
	
	// we don't want to draw a row until it is complete and time has moved on to the next row. other-
	// wise you end up with flickering on the top row as it gets updated.
	if (dif >= 1) {
		uint32_t *rowptr2;
		uint64_t j, row1;
		double value1;
		double high = gh->valhigh;
		double low = gh->vallow;
		
		row1 = gh->position - dif;
		rowptr2 = gh->data.data2 + ((gh->drawrow - 1) * width);
		
		for (i = 0; i < dif; ++i, ++row1) {
			rowptr1 = gh->data.data1 + ((row1%height)*width);
			
			for (j = 0; j < width; ++j) {
				value1 = *rowptr1;
				
				if (value1 >= high)
					*rowptr2 = *(gh->colors+steps_i-1);
				else if (value1 <= low)
					*rowptr2 = *gh->colors;
				else {
					*rowptr2 = *(gh->colors+(int)((value1/high)*steps_d));
				//*rowptr2 = *(gh->colors+(int)(((value1-low)/spread)*steps_d));
				}
				
				rowptr1++;
				rowptr2++;
			}
		}
		
		gh->drawrow -= dif;
		
		// if there's a redraw callback defined, tell them that there's new data to be drawn.
		if (gh->graph.redraw_fp != NULL)
			(*gh->graph.redraw_fp)((graph_t*)gh, gh->graph.redraw_ctx);
	}
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 */
inline graphhistory_t*
graphhistory_retain (graphhistory_t *gh)
{
	if (unlikely(gh == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null graphhistory_t");
	
	return (graphhistory_t*)graph_retain((graph_t*)gh);
}

/**
 *
 */
inline void
graphhistory_release (graphhistory_t *gh)
{
	if (unlikely(gh == NULL))
		LOG_ERROR_AND_RETURN(, "null graphhistory_t");
	
	graph_release((graph_t*)gh);
}
