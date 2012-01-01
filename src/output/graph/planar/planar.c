/*
 *  planar.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.24.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "planar.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRAPH_PLANAR_MAX_X 2048
#define GRAPH_PLANAR_MAX_Y 1000





#pragma mark -
#pragma mark private methods

static int __graphplanar_draw (graphplanar_t*, void*);
static int __graphplanar_feed (graphplanar_t*, uint32_t, double*);
static int __graphplanar_ready (graphplanar_t*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
graphplanar_init (graphplanar_t *gp, char *name, uint32_t size, uint8_t mode, opool_t *pool)
{
	int error;
	
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	// initialize the parent class
	if (unlikely(0 != (error = graph_init((graph_t*)gp, GRAPH_2D, name, size, (cobject_destroy_func)graphplanar_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to graph_init, %d", error);
	
	// the mode is a bitmask of cur, avg, min, max and indicates which lines will be drawn on the
	// graph.
	gp->mode = mode;
	
	// weightiness of the average
	gp->avg_size = 1000;
	
	// the function pointers are used by the parent class to tell us when we need to draw, process
	// new data or ready the graph (ie, allocate memory).
	gp->graph.__draw_fp = (__graph_draw_fp_func)__graphplanar_draw;
	gp->graph.__feed_fp = (__graph_feed_fp_func)__graphplanar_feed;
	gp->graph.__ready_fp = (__graph_ready_fp_func)__graphplanar_ready;
	
	return 0;
}

/**
 *
 *
 */
int
graphplanar_destroy (graphplanar_t *gp)
{
	int error;
	
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	// free the memory we allocated (if any)
	if (gp->data.data != NULL) {
		if (gp->data.path != NULL) {
			free(gp->data.path);
			gp->data.path = NULL;
		}
		
		free(gp->data.data);
		gp->data.data = NULL;
	}
	
	// clear the pointer in the parent class to our data
	gp->graph.data = NULL;
	
	// tell the parent class to clean up
	if (unlikely(0 != (error = graph_destroy((graph_t*)gp))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to graph_destroy, %d", gp->graph.name, error);
	
	return 0;
}

/**
 *
 *
 */
int
__graphplanar_ready (graphplanar_t *gp)
{
	uint32_t width, height;
	
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	// the width and height of the graph (in pixels)
	width = gp->graph.axis_x.length;
	height = gp->graph.axis_y.length;
	
	// limit the graph width to our maximum supported width
	if (unlikely(width > GRAPH_PLANAR_MAX_X))
		LOG_ERROR_AND_RETURN(-101, "[%s] width (%d) exceeds max (%d)", gp->graph.name, width, GRAPH_PLANAR_MAX_X);
	
	// limit the graph height to our maximum supported height
	if (unlikely(height > GRAPH_PLANAR_MAX_Y))
		LOG_ERROR_AND_RETURN(-102, "[%s] height (%d) exceeds max (%d)", gp->graph.name, height, GRAPH_PLANAR_MAX_Y);
	
	// allocate the "data" memory, which holds each point for each line
	if (unlikely(NULL == (gp->data.data = (graphplanar_point_t*)malloc(width * sizeof(graphplanar_point_t)))))
		LOG_ERROR_AND_RETURN(-103, "[%s] failed to malloc(%lu), %s", gp->graph.name, (width*sizeof(graphplanar_point_t)), strerror(errno));
	
	// allocate the "path" memory, which is where we temporarily copy data for a particulare line
	// so that we can call whoever is using us, and tell them to draw the given path, all at once,
	// instead of having to feed the individual points.
	if (unlikely(NULL == (gp->data.path = (float*)malloc((width+2) * sizeof(float)))))
		LOG_ERROR_AND_RETURN(-104, "[%s] failed to malloc(%lu), %s", gp->graph.name, ((width+2)*sizeof(float)), strerror(errno));
	
	// clear the "data" and "path" memory
	memset(gp->data.data, 0, (width*sizeof(graphplanar_point_t)));
	memset(gp->data.path, 0, ((width+2)*sizeof(float)));
	
	// note the number of bytes in the "path" so that we can easily (and correctly) clear that memory
	// elsewhere. yes, we could calculate it over and over, but that'd a maintenance nightmare.
	gp->data.path_size = (width+2) * sizeof(float);
	
	// give the parent class a pointer to our main graph data structure
	gp->graph.data = &gp->data;
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
inline graph_t*
graphplanar_graph (graphplanar_t *gp)
{
	return (graph_t*)gp;
}

/**
 *
 *
 */
int
graphplanar_mode_set (graphplanar_t *gp, uint8_t mode)
{
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	// replace the current mode bitmask with the new mode bitmask
	gp->mode = mode;
	
	return 0;
}

/**
 *
 *
 */
int
graphplanar_mode_get (graphplanar_t *gp, uint8_t *mode)
{
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	if (unlikely(mode == NULL))
		LOG_ERROR_AND_RETURN(-2, "null mode");
	
	// copy the current mode bitmask into the return pointer value thingy
	*mode = gp->mode;
	
	return 0;
}

/**
 * Replace the line and fill color settings for the given mode (ie, cur, avg, min, max) with the 
 * specified colors.
 *
 */
int
graphplanar_color_set (graphplanar_t *gp, graphplanar_mode mode, graph_color_t line, graph_color_t fill)
{
	graph_color_t *line_ptr, *fill_ptr;
	
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	// current
	if (GRAPH_PLANAR_MODE_CUR == mode) {
		line_ptr = &gp->cur_line;
		fill_ptr = &gp->cur_fill;
	}
	
	// average
	else if (GRAPH_PLANAR_MODE_AVG == mode) {
		line_ptr = &gp->avg_line;
		fill_ptr = &gp->avg_fill;
	}
	
	// minimum
	else if (GRAPH_PLANAR_MODE_MIN == mode) {
		line_ptr = &gp->min_line;
		fill_ptr = &gp->min_fill;
	}
	
	// maximum
	else if (GRAPH_PLANAR_MODE_MAX == mode) {
		line_ptr = &gp->max_line;
		fill_ptr = &gp->max_fill;
	}
	
	else
		LOG_ERROR_AND_RETURN(-101, "unsupported mode, 0x%04X", mode);
	
	// copy the new line and fill colors to the mode's line and fill colors
	memcpy(line_ptr, &line, sizeof(graph_color_t));
	memcpy(fill_ptr, &fill, sizeof(graph_color_t));
	
	return 0;
}

/**
 *
 *
 */
int
graphplanar_reset (graphplanar_t *gp)
{
	uint32_t width;
	
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	width = gp->graph.axis_x.length;
	
	// the graph has not been ready()'ed yet, so, don't reset anything
	if (width == 0 || gp->data.path == NULL || gp->data.data == NULL)
		return 0;
	
	memset(gp->data.data, 0, (width*sizeof(graphplanar_point_t)));
	memset(gp->data.path, 0, ((width+2)*sizeof(float)));
	
	dspchain_reset(&gp->graph.dspchain);
	
	return 0;
}





#pragma mark -
#pragma mark draw & feed

/**
 * Called by our parent class (graph_t) via the function pointer we defined in init(), when it is
 * time to draw the graph.
 *
 * We are given two callbacks that are either function pointer or block pointers - as indicated by
 * the "type" argument. The first callback is for drawing a path (ie, a set of points), and the
 * second callback is for drawing a line (ie, x1, y1, x2, y2).
 *
 * The "context" value is irrelevant to us; we just pass it to the callbacks.
 *
 */
static int
__graphplanar_draw (graphplanar_t *gp, void *context)
{
	uint32_t i, height, width;
	float *path;
	uint8_t mode = gp->mode;
	graphplanar_point_t *point;
	graph_draw_path_fp_func path_fp = (graph_draw_path_fp_func)gp->graph.draw_path_fp;
	graph_draw_line_fp_func line_fp = (graph_draw_line_fp_func)gp->graph.draw_line_fp;
	graph_draw_path_bp_func path_bp = (graph_draw_path_bp_func)gp->graph.draw_path_bp;
	graph_draw_line_bp_func line_bp = (graph_draw_line_bp_func)gp->graph.draw_line_bp;
	
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	// the width of the x axis (ie, the horizontal axis) in pixels
	height = gp->graph.axis_y.length;
	width = gp->graph.axis_x.length;
	
	
	
	// TODO: skip ahead to the "low" marker for the x axis, and stop at the "high" marker
	
	
	
	// draw the "maximum" line of the graph. the width of the path is the width of the axis plus two
	// additional points for the start and end of the path.
	if (GRAPH_PLANAR_MODE_MAX & mode)
	{
		point = gp->data.data;
		path = gp->data.path;
		
		// clear the path memory
		memset(gp->data.path, 0, gp->data.path_size);
		
		// first point of the path is 0,0
		path++;
		
		// for each point in the "current" line, copy its value to the path
		for (i = 0; i < width; ++i) {
			*path = (float)point->maxpt;
			
			path++;
			point++;
		}
		
		// draw the "maximum" fill area
		if (path_fp != NULL)
			(*path_fp)((graph_t*)gp, context, 1, gp->data.path, width+2, 0., 0., 1., (float)0.15);
		else if (path_bp != NULL)
			path_bp((graph_t*)gp, context, 1, gp->data.path, width+2, 0., 0., 1., (float)0.15);
		
		// draw the "maximum" line
		if (path_fp != NULL)
			(*path_fp)((graph_t*)gp, context, 0, gp->data.path, width+2, 1., 1., 1., .75);
		else if (path_bp != NULL)
			path_bp((graph_t*)gp, context, 0, gp->data.path, width+2, 1., 1., 1., .75);
	}
	
	
	
	
	
	// draw the "average" line of the graph. the width of the path is the width of the axis plus two
	// additional points for the start and end of the path.
	if (GRAPH_PLANAR_MODE_AVG & mode)
	{
		point = gp->data.data;
		path = gp->data.path;
		
		// clear the path memory
		memset(gp->data.path, 0, gp->data.path_size);
		
		// first point of the path is 0,0
		path++;
		
		// for each point in the "current" line, copy its value to the path
		for (i = 0; i < width; ++i) {
			*path = (float)point->avgpt;
			
			path++;
			point++;
		}
		
		// draw the "average" fill area
		if (path_fp != NULL)
			(*path_fp)((graph_t*)gp, context, 1, gp->data.path, width+2, 0., 1., 0., 0.25);
		else if (path_bp != NULL)
			path_bp((graph_t*)gp, context, 1, gp->data.path, width+2, 0., 1., 0., 0.25);
	}
	
	
	
	
	
	// draw the "current" line of the graph. the width of the path is the width of the axis plus two
	// additional points for the start and end of the path.
	if (GRAPH_PLANAR_MODE_CUR & mode)
	{
		point = gp->data.data;
		path = gp->data.path;
		
		// clear the path memory
		memset(gp->data.path, 0, gp->data.path_size);
		
		// first point of the path is 0,0
		path++;
		
		// for each point in the "current" line, copy its value to the path
		for (i = 0; i < width; ++i) {
			*path = (float)point->curpt;
			
			path++;
			point++;
		}
		
		// draw the "current" line
		if (path_fp != NULL)
			(*path_fp)((graph_t*)gp, context, 0, gp->data.path, width+2, 1., 0., 0., 1.);
		else if (path_bp != NULL)
			path_bp((graph_t*)gp, context, 0, gp->data.path, width+2, 1., 0., 0., 1.);
	}
	
	
	
	
	// draw the graph border
	if (line_fp != NULL) {
		(*line_fp)((graph_t*)gp, context, 0., 0., 0., height-1, 1., 1., 1., 1.);
		(*line_fp)((graph_t*)gp, context, 0., height-1, width-1, height-1, 1., 1., 1., 1.);
		(*line_fp)((graph_t*)gp, context, width-1, height-1, width-1, 0., 1., 1., 1., 1.);
		(*line_fp)((graph_t*)gp, context, width-1, 0., 0., 0., 1., 1., 1., 1.);
	}
	else if (line_bp != NULL) {
		line_bp((graph_t*)gp, context, 0., 0., 0., height-1, 1., 1., 1., 1.);
		line_bp((graph_t*)gp, context, 0., height-1, width-1, height-1, 1., 1., 1., 1.);
		line_bp((graph_t*)gp, context, width-1, height-1, width-1, 0., 1., 1., 1., 1.);
		line_bp((graph_t*)gp, context, width-1, 0., 0., 0., 1., 1., 1., 1.);
	}
	
	return 0;
}

/**
 *
 *
 */
static int
__graphplanar_feed (graphplanar_t *gp, uint32_t size, double *data)
{
	uint32_t i, count, height, avg_size;
	graphplanar_point_t *point;
	double cur, avg, min, max, val, low, sum, cnt, high, mul;
	int32_t curpt, avgpt, minpt, maxpt;
	graph_scale_type scale;
	
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-2, "null data");
	
	height = gp->graph.axis_y.length;
	point = gp->data.data;
	low = gp->graph.axis_y.trim_low;
	high = gp->graph.axis_y.trim_high;
	scale = gp->graph.axis_y.scale;
	count = size / sizeof(double);
	avg_size = gp->avg_size;
	mul = (double)height / (double)abs((int)(high - low));
	
	// if the data size exceeds the graph width, reduce the data size to the graph width
	if (count > gp->graph.axis_x.length)
		count = gp->graph.axis_x.length;
	
	// for each point in the feed data, update our current, average, minimum and maximum values for
	// that point. then update that graph point for each line, so that we don't have to do _any_
	// calculations when it comes time to draw.
	for (i = 0; i < count; ++i) {
		val = *data;
		avg = point->avg;
		min = point->min;
		max = point->max;
		sum = point->sum;
		cnt = point->cnt;
		
		// current
		cur = val;
		
		// average
		if (val > 0.0) {
			cnt += 1;
			
			if (cnt < avg_size) {
				sum = sum + val;
				avg = sum / cnt;
			}
			else {
				sum = (sum - avg) + val;
				avg = sum / (double)avg_size;
			}
		}
		
		// maximum
		if (val > max)
			max = val;
		
		// linear
		if (GRAPH_SCALE_LIN == scale) {
			curpt = (int32_t)((mul * cur) - low);
			avgpt = (int32_t)((mul * avg) - low);
			minpt = (int32_t)((mul * min) - low);
			maxpt = (int32_t)((mul * max) - low);
		}
		
		// exponential
		else if (GRAPH_SCALE_EXP == scale) {
			curpt = (int32_t)(log2(cur) - low);
			avgpt = (int32_t)(log2(avg) - low);
			minpt = (int32_t)(log2(min) - low);
			maxpt = (int32_t)(log2(max) - low);
		}
		
		// logarithmic
		else if (GRAPH_SCALE_LOG == scale) {
			curpt = (int32_t)(log(cur) - low);
			avgpt = (int32_t)(log(avg) - low);
			minpt = (int32_t)(log(min) - low);
			maxpt = (int32_t)(log(max) - low);
		}
		
		else
			LOG_ERROR_AND_RETURN(-101, "unsupported scale, 0x%X", scale);
		
		// don't let them draw off the screen
		if (curpt < 0) curpt = 0;
		if (avgpt < 0) avgpt = 0;
		if (minpt < 0) minpt = 0;
		if (maxpt < 0) maxpt = 0;
		
		// copy the new graph line point values back into the point structure
		point->curpt = (uint32_t)curpt;
		point->avgpt = (uint32_t)avgpt;
		point->minpt = (uint32_t)minpt;
		point->maxpt = (uint32_t)maxpt;
		
		// copy the new graph values back into the graph point
		point->cur = cur;
		point->avg = avg;
		point->min = min;
		point->max = max;
		point->sum = sum;
		point->cnt = cnt;
		
		// move the pointer forward to the next point structure in the graph
		point++;
		
		// move the pointer forward to the next value in the input data
		data++;
	}
	
	// if there's a redraw callback defined, tell them that there's new data to be drawn.
	if (gp->graph.redraw_fp != NULL)
		(*gp->graph.redraw_fp)((graph_t*)gp, gp->graph.redraw_ctx);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline graphplanar_t*
graphplanar_retain (graphplanar_t *gp)
{
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null graphplanar_t");
	
	return (graphplanar_t*)graph_retain((graph_t*)gp);
}

/**
 *
 *
 */
inline void
graphplanar_release (graphplanar_t *gp)
{
	if (unlikely(gp == NULL))
		LOG_ERROR_AND_RETURN(, "null graphplanar_t");
	
	graph_release((graph_t*)gp);
}
