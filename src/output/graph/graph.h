/*
 *  graph.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.24.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  The graph_t class is subclassed by an actual graph implementation. The elements here are common
 *  to all graphs. Use graph_init() (indirectly via the subclass) and graph_set_axis() to configure 
 *  the graph, in addition, provide the necessary callbacks (as required by the particular graph).
 *  Finally, start graph_feed()'ing data to the graph and it'll start calling the various draw
 *  callbacks.
 *
 *  If the data requires some pre-processing before it can be suitably graphed, configure the
 *  dspchain accordingly. For instance, as of the time of this writing, the existing graphs require
 *  doubles. Use the dsp class short2double_t, for instance, to convert your data.
 *
 */

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <stdint.h>
#include "../dataobject.h"
#include "../datastream.h"
#include "../../dsp/dspchain.h"
#include "../../misc/mem/cobject.h"
#include "../../misc/mem/opool.h"
#include "../../misc/thread/mutex.h"

//
// graph_type
//
typedef enum
{
	GRAPH_2D = (1 << 0),                 // two dimensional
	GRAPH_3D = (1 << 1)                  // three dimensional
} graph_type;

//
// graph_axis
//
typedef enum
{
	GRAPH_AXIS_X = (1 << 0),             // x axis
	GRAPH_AXIS_Y = (1 << 1),             // y axis
	GRAPH_AXIS_Z = (1 << 2)              // z axis
} graph_axis_type;

//
// graph_scale
//
typedef enum
{
	GRAPH_SCALE_LIN = (1 << 0),          // linear
	GRAPH_SCALE_EXP = (1 << 1),          // exponential
	GRAPH_SCALE_LOG = (1 << 2)           // logarithmic
} graph_scale_type;

//
// graph_color
//
struct graph_color
{
	float r;                             // red
	float g;                             // green
	float b;                             // blue
	float a;                             // alpha
};
typedef struct graph_color graph_color_t;

//
// graph_rect
//
struct graph_rect
{
	// x, y, w, h
};
typedef struct graph_rect graph_rect_t;

//
// graph_value
//
struct graph_value
{
	uint32_t xloc;                       // x-axis relative location
	uint32_t yloc;                       // y-axis relative location
	double xaxis;                        // x-axis absolute location
	double yaxis;                        // y-axis absolute location
	double xval;                         // x-axis value
	double yval;                         // y-axis value
};
typedef struct graph_value graph_value_t;

//
// graph_axis
//
struct graph_axis
{
	char name[50];                       // axis name
	graph_axis_type axis;                // graph axis (x, y, z)
	graph_scale_type scale;              // linear, exp, log
	double trim_high;                    // ignore values above this point
	double trim_low;                     // ignore values below this point
	double unit_high;                    // high value on the axis
	double unit_low;                     // low value on the axis
	char unit_name[50];                  // Hz, KHz, MHz, Elmo, etc.
	uint32_t length;                     // data width/height
};
typedef struct graph_axis graph_axis_t;

//
// graph
//
struct graph
{
	cobject_t cobject;                   // parent class
	
	char name[50];                       // graph name
	graph_type type;                     // axis count
	void *data;                          // raw graph data
	uint32_t ready;                      // graph is ready to use
	uint64_t samples;                    // number of samples
	
	mutex_t mutex;                       // data lock
	
	datastream_t datastream;             // data stream
	dspchain_t dspchain;                 // dsp chain
	
	graph_axis_t axis_x;                 // axis x
	graph_axis_t axis_y;                 // axis y
	graph_axis_t axis_z;                 // axis z
	
	/* draw path */
	int (*draw_path_fp)(struct graph*, void*, int, float*, uint32_t, float, float, float, float);
	int (^draw_path_bp)(struct graph*, void*, int, float*, uint32_t, float, float, float, float);
	
	/* draw line */
	int (*draw_line_fp)(struct graph*, void*, float, float, float, float, float, float, float, float);
	int (^draw_line_bp)(struct graph*, void*, float, float, float, float, float, float, float, float);
	
	/* draw rect */
	int (*draw_rect_fp)(struct graph*, void*, graph_rect_t*);
	int (^draw_rect_bp)(struct graph*, void*, graph_rect_t*);
	
	/* draw text */
	int (*draw_text_fp)(struct graph*, void*, float, float, char*);
	int (^draw_text_bp)(struct graph*, void*, float, float, char*);
	
	/* draw geom */
	int (*draw_geom_fp)(struct graph*, void*);
	int (^draw_geom_bp)(struct graph*, void*);
	
	/* draw hist */
	int (*draw_hist_fp)(struct graph*, void*, uint32_t*, uint32_t, uint32_t);
	int (^draw_hist_bp)(struct graph*, void*, uint32_t*, uint32_t, uint32_t);
	
	/* redraw */
	void *redraw_ctx;									// redraw context
	int (*redraw_fp)(struct graph*, void*);
	int (^redraw_bp)(struct graph*, void*);
	
	/* private: draw */
	int (*__draw_fp)(struct graph*, void*);
	int (^__draw_bp)(struct graph*, void*);
	
	/* private: feed */
	int (*__feed_fp)(struct graph*, uint32_t, void*);
	int (^__feed_bp)(struct graph*, uint32_t, void*);
	
	/* private: ready */
	int (*__ready_fp)(struct graph*);
	int (^__ready_bp)(struct graph*);
};
typedef struct graph graph_t;





//
// graph_draw_path_fp_func
// graph_draw_path_bp_func
//
// graph, context, fill (?), points, count, r, g, b, a
//
typedef int (*graph_draw_path_fp_func)(graph_t*, void*, int, float*, uint32_t, float, float, float, float);
typedef int (^graph_draw_path_bp_func)(graph_t*, void*, int, float*, uint32_t, float, float, float, float);

//
// graph_draw_line_fp_func
// graph_draw_line_bp_func
//
// graph, context, x1, y1, x2, y2, r, g, b, a
//
typedef int (*graph_draw_line_fp_func)(graph_t*, void*, float, float, float, float, float, float, float, float);
typedef int (^graph_draw_line_bp_func)(graph_t*, void*, float, float, float, float, float, float, float, float);

//
// graph_draw_rect_fp_func
// graph_draw_rect_bp_func
//
typedef int (*graph_draw_rect_fp_func)();
typedef int (^graph_draw_rect_bp_func)();

//
// graph_draw_text_fp_func
// graph_draw_text_bp_func
//
// graph, context, x, y, text
//
typedef int (*graph_draw_text_fp_func)(graph_t*, void*, float, float, char*);
typedef int (^graph_draw_text_bp_func)(graph_t*, void*, float, float, char*);

//
// graph_draw_hist_fp_func
// graph_draw_hist_bp_func
//
// graph, context, data, width, height
//
typedef int (*graph_draw_hist_fp_func)(graph_t*, void*, uint32_t*, uint32_t, uint32_t);
typedef int (^graph_draw_hist_bp_func)(graph_t*, void*, uint32_t*, uint32_t, uint32_t);

//
// graph_redraw_fp_func
// graph_redraw_bp_func
//
// graph, context
//
typedef int (*graph_redraw_fp_func)(graph_t*, void*);
typedef int (^graph_redraw_bp_func)(graph_t*, void*);

//
// __graph_draw_fp_func
// __graph_draw_bp_func
//
// graph, context
//
typedef int (*__graph_draw_fp_func)(graph_t*, void*);
typedef int (^__graph_draw_bp_func)(graph_t*, void*);

//
// __graph_feed_fp_func
// __graph_feed_bp_func
//
// graph, size, data
//
typedef int (*__graph_feed_fp_func)(struct graph*, uint32_t, void*);
typedef int (^__graph_feed_bp_func)(struct graph*, uint32_t, void*);

//
// __graph_ready_fp_func
// __graph_ready_bp_func
//
// graph
//
typedef int (*__graph_ready_fp_func)(struct graph*);
typedef int (^__graph_ready_bp_func)(struct graph*);





/**
 * graph, name, data block size, axis count
 */
int graph_init (graph_t*, graph_type, char*, uint32_t, cobject_destroy_func, opool_t*);

/**
 * Do not call this directly. When the last reference to the graph is released, the memory manager
 * will call the *_destroy() function of the most-derived class, which has as its responsibility to
 * call the *_destroy() function of its parent class, all the way up.
 *
 * Only a class deriving from graph_t should call this function directly.
 */
int graph_destroy (graph_t*);

/**
 * After everything is configured, tell the graph to set itself up.
 */
int graph_ready (graph_t*);





/**
 * Sets the draw-path callback. The graph uses this callback to draw a closed path. It is up to the
 * underlying interface to connect the first and last points (if the graphics library requires it).
 */
int graph_set_draw_path_fp (graph_t*, graph_draw_path_fp_func);
int graph_set_draw_path_bp (graph_t*, graph_draw_path_bp_func);

/**
 * Sets the draw-line callback. The graph uses this callback to draw a single line segment.
 */
int graph_set_draw_line_fp (graph_t*, graph_draw_line_fp_func);
int graph_set_draw_line_bp (graph_t*, graph_draw_line_bp_func);

/**
 * Sets the draw-hist callback. The graph uses this callback to draw a two-dimensional RGBA image
 * with an origin (ie, 0,0) in the top-left corner.
 */
int graph_set_draw_hist_fp (graph_t*, graph_draw_hist_fp_func);
int graph_set_draw_hist_bp (graph_t*, graph_draw_hist_bp_func);

/**
 * Sets the redraw callback. The graph uses this callback when its data is updated such that it can
 * notify the implementing interface that it needs to redraw itself (by calling graph_redraw()).
 */
int graph_set_redraw_fp (graph_t*, graph_redraw_fp_func, void*);
int graph_set_redraw_bp (graph_t*, graph_redraw_bp_func, void*);





/**
 * graph, axis, name, scale, trim high, trim low, unit high, unit low, data size (ie, width)
 */
int graph_set_axis (graph_t*, graph_axis_type, char*, graph_scale_type, double, double, double, double, uint32_t);

/**
 * graph, axis
 */
int graph_set_axis2 (graph_t*, graph_axis_t);

/**
 * Feed the graph some data.
 */
int graph_feed (graph_t*, uint32_t, void*);

/**
 * Get the graph's datastream object.
 */
int graph_datastream (graph_t*, datastream_t**);

/**
 * Get the dspchain_t instance for this graph.
 */
int graph_dspchain (graph_t*, dspchain_t**);

/**
 * graph, x, y, value
 *
 * Get information from the graph at a particular location.
 */
int graph_get_value (graph_t*, uint32_t, uint32_t, graph_value_t*);





/**
 * Get a pointer to the "raw" graph data.
 */
int graph_data (graph_t*, void**);

/**
 * Draw the graph using the previously specified draw callback function, passing the given context.
 */
int graph_draw (graph_t*, void*);





/**
 * Current time in milliseconds.
 */
uint64_t graph_milliseconds ();





/**
 * Retain the graph.
 */
graph_t* graph_retain (graph_t*);

/**
 * Release the graph.
 */
void graph_release (graph_t*);

#endif /* __GRAPH_H__ */
