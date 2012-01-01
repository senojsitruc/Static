/*
 *  planar.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.24.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  We probably want to add some decaying average support to this.
 *
 *  And line smoothing. Starting with the first point, find the highest close point (ie, within just
 *  a few columns) and draw a straight line between the first point and that next high point.
 *
 *  TODO: add an option to invert
 *
 */

#ifndef __GRAPH_PLANAR_H__
#define __GRAPH_PLANAR_H__

#include "../graph.h"
#include "../../../misc/mem/opool.h"

//
// graphplanar_mode
//
typedef enum
{
	GRAPH_PLANAR_MODE_CUR = (1 << 0),    // current
	GRAPH_PLANAR_MODE_AVG = (1 << 1),    // average
	GRAPH_PLANAR_MODE_MIN = (1 << 2),    // minimum
	GRAPH_PLANAR_MODE_MAX = (1 << 3)     // maximum
} graphplanar_mode;

//
// graphplanar_point
//
struct graphplanar_point
{
	/* stores the actual data values */
	double cur;                          // current value
	double avg;                          // average value
	double min;                          // minimum value
	double max;                          // maximum value
	double sum;                          // sum value for averaging
	double cnt;                          // sample count for averaging
	
	/* stores the graph points */
	uint32_t curpt;                      // current point
	uint32_t avgpt;                      // average point
	uint32_t minpt;                      // minimum point
	uint32_t maxpt;                      // maximum point
};
typedef struct graphplanar_point graphplanar_point_t;

//
// graphplanar_data
//
struct graphplanar_data
{
	graphplanar_point_t *data;           // data points
	float *path;                         // path scratch space
	uint32_t path_size;                  // path size in bytes
};
typedef struct graphplanar_data graphplanar_data_t;

//
// graphplanar
//
struct graphplanar
{
	graph_t graph;                       // parent class
	uint8_t mode;                        // display mode (cur, avg, min, max)
	graphplanar_data_t data;             // graph data
	
	uint32_t avg_size;                   // weightiness of the average
	
	graph_color_t cur_line;              // current - line color
	graph_color_t cur_fill;              // current - fill color
	graph_color_t avg_line;              // average - line color
	graph_color_t avg_fill;              // average - fill color
	graph_color_t min_line;              // minimum - line color
	graph_color_t min_fill;              // minimum - fill color
	graph_color_t max_line;              // maximum - line color
	graph_color_t max_fill;              // maximum - fill color
};
typedef struct graphplanar graphplanar_t;





/**
 * graph, name, data block size, display mode, pool
 */
int graphplanar_init (graphplanar_t*, char*, uint32_t, uint8_t, opool_t*);

/**
 * graph
 */
int graphplanar_destroy (graphplanar_t*);





/**
 *
 */
graph_t* graphplanar_graph (graphplanar_t*);

/**
 *
 */
int graphplanar_mode_set (graphplanar_t*, uint8_t);

/**
 *
 */
int graphplanar_mode_get (graphplanar_t*, uint8_t*);

/**
 * graph, mode (cur, avg, min, max), line color, fill color
 */
int graphplanar_color_set (graphplanar_t*, graphplanar_mode, graph_color_t, graph_color_t);

/**
 *
 */
int graphplanar_reset (graphplanar_t*);





/**
 *
 */
graphplanar_t* graphplanar_retain (graphplanar_t*);

/**
 *
 */
void graphplanar_release (graphplanar_t*);

#endif /* __GRAPH_PLANA_H__ */
