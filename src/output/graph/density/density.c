/*
 *  density.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.24.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "density.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRAPH_DENSITY_MAX_X 2048
#define GRAPH_DENSITY_MAX_Y 1000





#pragma mark -
#pragma mark private methods

static int __graphdensity_draw (graphdensity_t*, void*);
static int __graphdensity_feed (graphdensity_t*, uint32_t, double*);
static int __graphdensity_ready (graphdensity_t*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
graphdensity_init (graphdensity_t *gd, char *name, uint32_t size, opool_t *pool)
{
	int error;
	
	if (unlikely(gd == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphdensity_t");
	
	// initialize the parent class
	if (unlikely(0 != (error = graph_init((graph_t*)gd, GRAPH_2D, name, size, (cobject_destroy_func)graphdensity_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to graph_init, %d", name, error);
	
	// the function pointers are used by the parent class to tell us when we need to draw, process
	// new data or ready the graph (ie, allocate memory).
	gd->graph.__draw_fp = (__graph_draw_fp_func)__graphdensity_draw;
	gd->graph.__feed_fp = (__graph_feed_fp_func)__graphdensity_feed;
	gd->graph.__ready_fp = (__graph_ready_fp_func)__graphdensity_ready;
	
	return 0;
}

/**
 *
 *
 */
int
graphdensity_destroy (graphdensity_t *gd)
{
	int error;
	
	// TODO: free
	
	gd->graph.data = NULL;
	
	if (unlikely(0 != (error = graph_destroy((graph_t*)gd))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to graph_destroy, %d", gd->graph.name, error);
	
	
	
	
	return 0;
}

/**
 *
 *
 */
static int
__graphdensity_ready (graphdensity_t *gd)
{
	uint32_t width, height;
	
	if (unlikely(gd == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphdensity_t");
	
	// the width and height of the graph (in pixels)
	width = gd->graph.axis_x.length;
	height = gd->graph.axis_y.length;
	
	// limit the graph width to our maximum supported width
	if (unlikely(width > GRAPH_DENSITY_MAX_X))
		LOG_ERROR_AND_RETURN(-101, "[%s] width (%d) exceeds max (%d)", gd->graph.name, width, GRAPH_DENSITY_MAX_X);
	
	// limit the graph height to our maximum supported height
	if (unlikely(height > GRAPH_DENSITY_MAX_Y))
		LOG_ERROR_AND_RETURN(-102, "[%s] height (%d) exceeds max (%d)", gd->graph.name, height, GRAPH_DENSITY_MAX_Y);
	
	// allocate the memory used to hold the graph data
	if (unlikely(NULL == (gd->data.data = (uint64_t*)malloc(width * height * sizeof(uint64_t)))))
		LOG_ERROR_AND_RETURN(-103, "[%s] failed to malloc(%lu), %s", gd->graph.name, (width*height*sizeof(uint64_t)), strerror(errno));
	
	// clear the memory used to hold the graph data
	memset(gd->data.data, 0, (width*height*sizeof(uint64_t)));
	
	// give the parent class a pointer to our main graph data structure
	gd->graph.data = &gd->data;
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
graph_t*
graphdensity_graph (graphdensity_t *gd)
{
	return (graph_t*)gd;
}





#pragma mark -
#pragma mark draw & feed

/**
 *
 *
 */
static int
__graphdensity_draw (graphdensity_t *gd, void *context)
{
	/*
	int error, x, y, width, height;
	uint64_t *point, val;
	double multiplier;
	graph_color_t *colors;
	graph_rect_t rect;
	graph_draw_path_fp_func path_fp = (graph_draw_path_fp_func)path_cb;
	graph_draw_line_fp_func line_fp = (graph_draw_line_fp_func)line_cb;
	graph_draw_path_bp_func path_bp = (graph_draw_path_bp_func)path_cb;
	graph_draw_line_bp_func line_bp = (graph_draw_line_bp_func)line_cb;
	
	if (unlikely(gd == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphdensity_t");
	
	// the callback type must be one of the two supported types (ie, function or block)
	if (unlikely(type != GRAPH_CB_FP && type != GRAPH_CB_BP))
		LOG_ERROR_AND_RETURN(-2, "invalid callback type, 0x%04X", type);
	
	// the draw-path callback must be specified
	if (unlikely(path_cb == NULL))
		LOG_ERROR_AND_RETURN(-3, "null path callback");
	
	// the draw-line callback must be specified
	if (unlikely(line_cb == NULL))
		LOG_ERROR_AND_RETURN(-4, "null line callback");
	
	// the height (of the y axis) and the width (of the x axis)
	width = gp->graph.axis_x.width;
	height = gp->graph.axis_y.width;
	
	// this point represents the density value for a particalar location in the graph
	point = gd->data.data;
	
	// get the color spectrum from the graph; we'll use this to pick color values
	colors = gd->graph.colors;
	
	// the multiplier is used to scale a value into the graph's color spectrum. the scale factor is
	// determined by using the current maximum value in the graph to the represent the highest 
	// possible color.
	multiplier = (double)gd->maxval / (double)gd->graph.colorcnt;
	
	// for each point in the graph, iterating from the bottom left to the top right, going vertical
	// first, and then horizontal.
	for (x = 0; x < width; ++x) {
		for (y = 0; y < height; ++y) {
			val = *point;
			
			graph_color_t *color = color + ((double)val * multiplier);
			
			rect.x = x;
			rect.y = y;
			rect.w = 1.;
			rect.h = 1.;
			
			// call rect() calback
			
			point++;
		}
	}
	*/
	
	
	
	return 0;
}

/**
 *
 *
 */
static int
__graphdensity_feed (graphdensity_t *gd, uint32_t size, double *data)
{
	uint32_t i, width, height;
	uint64_t *point, cur, max;
	double val, low, high;
	graph_scale_type scale;
	
	if (unlikely(gd == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphdensity_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-2, "null data");
	
	width = gd->graph.axis_x.length;
	height = gd->graph.axis_y.length;
	scale = gd->graph.axis_x.scale;
	point = gd->data.data;
	low = gd->graph.axis_y.trim_low;
	high = gd->graph.axis_y.trim_high;
	max = gd->maxval;
	
	for (i = 0; i < width; ++i) {
		val = *data;
		
		// linear
		if (GRAPH_SCALE_LIN == scale)
			val -= low;
		
		// exponential
		else if (GRAPH_SCALE_EXP == scale)
			val = log2(val) - low;
		
		// logarithmic
		else if (GRAPH_SCALE_LOG == scale)
			val = log(val) - low;
		
		// if the value is within range of the graph, increment the value at that location. also, if the
		// new value is greater than the current maximum value across the entire graph, set the new 
		// maximum value.
		if (val < high && val > low) {
			cur = *(point+(long)val) + 1;
			
			if (cur > max)
				max = cur;
			
			*(point+(long)val) = cur;
		}
		
		// move the pointer forward to the next column in the graph
		point += height;
		
		// move the pointer forward to the next value in the input data
		data++;
	}
	
	gd->maxval = max;
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 */
inline graphdensity_t*
graphdensity_retain (graphdensity_t *gd)
{
	if (unlikely(gd == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null graphdensity_t");
	
	return (graphdensity_t*)graph_retain((graph_t*)gd);
}

/**
 *
 */
inline void
graphdensity_release (graphdensity_t *gd)
{
	if (unlikely(gd == NULL))
		LOG_ERROR_AND_RETURN(, "null graphdensity_t");
	
	graph_release((graph_t*)gd);
}
