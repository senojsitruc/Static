/*
 *  graph.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.24.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "graph.h"
#include "../dataobject.h"
#include "../datastream.h"
#include "../../misc/logger.h"
#include <string.h>
#include <sys/time.h>





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
graph_init (graph_t *graph, graph_type type, char *name, uint32_t size, cobject_destroy_func destroy, opool_t *pool)
{
	int error;
	size_t name_l;
	
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(name == NULL || (name_l = strlen(name)) == 0))
		LOG_ERROR_AND_RETURN(-2, "null or zero-length name");
	
	if (unlikely(name_l >= sizeof(graph->name)))
		LOG_ERROR_AND_RETURN(-3, "name is too long (max=%lu)", (sizeof(graph->name)-1));
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)graph, name, (cobject_destroy_func)destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	if (unlikely(0 != (error = datastream_init(&graph->datastream, graph, size, (datastream_feed_func)graph_feed))))
		LOG_ERROR_AND_RETURN(-102, "failed to datastream_init, %d", error);
	
	if (unlikely(0 != (error = dspchain_init(&graph->dspchain, 10, pool))))
		LOG_ERROR_AND_RETURN(-103, "failed to dspchain_init, %d", error);
	
	if (unlikely(0 != (error = mutex_init(&graph->mutex, LOCK_TYPE_SPIN))))
		LOG_ERROR_AND_RETURN(-104, "failed to mutex_init, %d", error);
	
	graph->type = type;
	memcpy(graph->name, name, name_l);
	
	return 0;
}

/**
 *
 *
 */
int
graph_destroy (graph_t *graph)
{
	int error;
	
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(0 != (error = mutex_destroy(&graph->mutex))))
		LOG_ERROR_AND_RETURN(-101, "failed to mutex_destroy, %d", error);
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)graph))))
		LOG_ERROR_AND_RETURN(-102, "failed to cobject_destroy, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
graph_ready (graph_t *graph)
{
	int error;
	
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (graph->ready == 1)
		LOG_ERROR_AND_RETURN(-101, "graph is already ready");
	
	if (graph->__ready_fp != NULL) {
		if (unlikely(0 != (error = (*graph->__ready_fp)(graph))))
			LOG_ERROR_AND_RETURN(-102, "[%s] failed to graph->__ready_fp, %d", graph->name, error);
	}
	
	else if (graph->__ready_bp != NULL) {
		if (unlikely(0 != (error = graph->__ready_bp(graph))))
			LOG_ERROR_AND_RETURN(-103, "[%s] failed to graph->__ready_bp, %d", graph->name, error);
	}
	
	else
		LOG_ERROR_AND_RETURN(-104, "[%s] no ready callback", graph->name);
	
	graph->ready = 1;
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 * Get a pointer to the "raw" graph data.
 *
 */
int
graph_data (graph_t *graph, void **data)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-2, "[%s] null return data poniter", graph->name);
	
	*data = graph->data;
	
	return 0;
}

/**
 *
 *
 */
int
graph_draw (graph_t *graph, void *context)
{
	int error = 0;
	
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	//mutex_lock(&graph->mutex);
	
	// call derived class function pointer
	if (graph->__draw_fp != NULL) {
		if (unlikely(0 != (error = (*graph->__draw_fp)(graph, context))))
			LOG_ERROR_AND_GOTO(-101, done, "[%s] failed to graph->__draw_fp, %d", graph->name, error);
	}
	
	// call derived call block pointer
	else if (graph->__draw_bp != NULL) {
		if (unlikely(0 != (error = graph->__draw_bp(graph, context))))
			LOG_ERROR_AND_GOTO(-102, done, "[%s] failed to graph->__draw_bp, %d", graph->name, error);
	}
	
	// fail
	else
		LOG_ERROR_AND_GOTO(-103, done, "[%s] no draw callback", graph->name);
	
done:
//mutex_unlock(&graph->mutex);
	return error;
}

/**
 *
 *
 */
int
graph_feed (graph_t *graph, uint32_t size, void *data)
{
	int error = 0;
	
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(size == 0))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	// run the sample data through the dsp chain
	if (unlikely(0 != (error = dspchain_run(&graph->dspchain, &size, data))))
		LOG_ERROR_AND_RETURN(-101, "failed to dspchain_run, %d", error);
	
	mutex_lock(&graph->mutex);
	
	// some of the dsp processes may require a certain number of samples before they result in usable
	// data for the dsp processes following them, or the output mechanisms (ie, graphs). for this 
	// reason a dsp might set the data size to zero until it has a sufficient number of samples.
	if (size != 0) {
		if (graph->__feed_fp != NULL) {
			if (unlikely(0 != (error = (*graph->__feed_fp)(graph,size,data))))
				LOG_ERROR_AND_GOTO(-102, done, "[%s] failed to graph->feed_fp, %d", graph->name, error);
		}
		else if (graph->__feed_bp != NULL) {
			if (unlikely(0 != (error = graph->__feed_bp(graph,size,data))))
				LOG_ERROR_AND_GOTO(-103, done, "[%s] failed to graph->feed_bp, %d", graph->name, error);
		}
		else
			LOG_ERROR_AND_GOTO(-104, done, "[%s] no feed callback", graph->name);
	}
	
	graph->samples++;
	
done:
	mutex_unlock(&graph->mutex);
	return error;
}

/**
 *
 *
 */
int
graph_datastream (graph_t *graph, datastream_t **datastream)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(datastream == NULL))
		LOG_ERROR_AND_RETURN(-2, "null datastream_t");
	
	*datastream = &graph->datastream;
	
	return 0;
}

/**
 *
 *
 */
int
graph_dspchain (graph_t *graph, dspchain_t **dspchain)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-2, "null dspchain_t");
	
	*dspchain = &graph->dspchain;
	
	return 0;
}

/**
 *
 *
 */
int
graph_get_value (graph_t *graph, uint32_t x, uint32_t y, graph_value_t *value)
{
	graph_axis_t *xaxis, *yaxis;
	
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(value == NULL))
		LOG_ERROR_AND_RETURN(-2, "null graph_value_t");
	
	xaxis = &graph->axis_x;
	yaxis = &graph->axis_y;
	
	value->xloc = x;
	value->yloc = y;
	value->xaxis = xaxis->unit_low + (((double)x / (double)xaxis->length) * (xaxis->unit_high-xaxis->unit_low));
	value->yaxis = yaxis->unit_low + (((double)y / (double)yaxis->length) * (yaxis->unit_high-yaxis->unit_low));
	value->xval = 0.;
	value->yval = 0.;
	
	// TODO: the graph value can only be obtained by calling the subclass's callback:
	//       (*graph->__value_fp)(graph, x, y, value);
	
	return 0;
}





#pragma mark -
#pragma mark accessors - callbacks

/**
 *
 *
 *
 */
int
graph_set_draw_path_fp (graph_t *graph, graph_draw_path_fp_func func)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	graph->draw_path_fp = func;
	graph->draw_path_bp = NULL;
	
	return 0;
}

/**
 *
 *
 *
 */
int
graph_set_draw_path_bp (graph_t *graph, graph_draw_path_bp_func func)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	graph->draw_path_fp = NULL;
	graph->draw_path_bp = func;
	
	return 0;
}

/**
 *
 *
 *
 */
int
graph_set_draw_line_fp (graph_t *graph, graph_draw_line_fp_func func)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	graph->draw_line_fp = func;
	graph->draw_line_bp = NULL;
	
	return 0;
}

/**
 *
 *
 *
 */
int
graph_set_draw_line_bp (graph_t *graph, graph_draw_line_bp_func func)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	graph->draw_line_fp = NULL;
	graph->draw_line_bp = func;
	
	return 0;
}

/**
 *
 *
 */
int
graph_set_draw_hist_fp (graph_t *graph, graph_draw_hist_fp_func func)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	graph->draw_hist_fp = func;
	graph->draw_hist_bp = NULL;
	
	return 0;
}

/**
 *
 *
 */
int
graph_set_draw_hist_bp (graph_t *graph, graph_draw_hist_bp_func func)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	graph->draw_hist_fp = NULL;
	graph->draw_hist_bp = func;
	
	return 0;
}

/**
 *
 *
 */
int
graph_set_redraw_fp (graph_t *graph, graph_redraw_fp_func func, void *context)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	graph->redraw_fp = func;
	graph->redraw_bp = NULL;
	graph->redraw_ctx = context;
	
	return 0;
}

/**
 *
 *
 */
int
graph_set_redraw_bp (graph_t *graph, graph_redraw_bp_func func, void *context)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	graph->redraw_fp = NULL;
	graph->redraw_bp = func;
	graph->redraw_ctx = context;
	
	return 0;
}





#pragma mark -
#pragma mark accessors - axes

/**
 *
 *
 */
int
graph_set_axis (graph_t *graph, graph_axis_type axis, char *name, graph_scale_type scale, double trim_high, double trim_low, double unit_high, double unit_low, uint32_t length)
{
	size_t name_l;
	graph_axis_t *ga;
	
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(name == NULL || (name_l = strlen(name)) == 0))
		LOG_ERROR_AND_RETURN(-2, "[%s] null or zero-length name", graph->name);
	
	if (GRAPH_AXIS_X == axis)
		ga = &graph->axis_x;
	else if (GRAPH_AXIS_Y == axis)
		ga = &graph->axis_y;
	else if (GRAPH_AXIS_Z == axis)
		ga = &graph->axis_z;
	else
		LOG_ERROR_AND_RETURN(-102, "[%s] unknown axis, %d", graph->name, axis);
	
	ga->axis = axis;
	ga->scale = scale;
	ga->trim_high = trim_high;
	ga->trim_low = trim_low;
	ga->unit_high = unit_high;
	ga->unit_low = unit_low;
	ga->length = length;
	
	return 0;
}

/**
 *
 *
 */
int
graph_set_axis2 (graph_t *graph, graph_axis_t axis)
{
	graph_axis_t *ga;
	
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (GRAPH_AXIS_X == axis.axis)
		ga = &graph->axis_x;
	else if (GRAPH_AXIS_Y == axis.axis)
		ga = &graph->axis_y;
	else if (GRAPH_AXIS_Z == axis.axis)
		ga = &graph->axis_z;
	else
		LOG_ERROR_AND_RETURN(-102, "[%s] unknown axis, %d", graph->name, axis.axis);
	
	memcpy(ga, &axis, sizeof(graph_axis_t));
	
	return 0;
}





#pragma mark -
#pragma mark miscellaneous

/**
 * Time in milliseconds.
 *
 */
inline uint64_t
graph_milliseconds ()
{
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	
	return (((uint64_t)tv.tv_sec * 1000000) + (uint64_t)tv.tv_usec) / 1000;
}





#pragma mark -
#pragma mark cobject stuff

/**
 * Retain the graph.
 *
 */
inline graph_t*
graph_retain (graph_t *graph)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null graph_t");
	
	return (graph_t*)cobject_retain((cobject_t*)graph);
}

/**
 * Release the graph.
 *
 */
inline void
graph_release (graph_t *graph)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(, "null graph_t");
	
	cobject_release((cobject_t*)graph);
}
