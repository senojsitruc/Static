/*
 *  history.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.24.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  
 *
 */

#ifndef __GRAPH_HISTORY_H__
#define __GRAPH_HISTORY_H__

#include "../graph.h"
#include "../../../misc/mem/opool.h"

//
// graphhistory_data
//
struct graphhistory_data
{
	uint32_t size1;                      // size of data1 (in bytes)
	uint32_t size2;                      // size of data2 (in bytes)
	uint32_t count;                      // number of uint64_t's
	double *data1;                       // graph data
	uint32_t *data2;                     // graph data (in colors)
};
typedef struct graphhistory_data graphhistory_data_t;

//
// graphhistory
//
struct graphhistory
{
	graph_t graph;                       // parent class
	graphhistory_data_t data;            // graph data
	
	uint64_t datarow;                    // 
	uint64_t drawrow;                    // 
	uint64_t drawrows;                   // height of draw area (not just the visible graph)
	
	uint64_t starttime;                  // start time (in milliseconds)
	
	uint64_t position;                   // current row in the waterfall
	uint64_t positioncnt;                // number of samples in current row
	uint64_t drawposition;               // last drawn row in the waterfall
	
	double valhigh;                      // high value for calibrating the graph
	double vallow;                       // low value for calibrating the graph
	
	uint32_t colorsteps;                 // number of colors
	uint32_t *colors;                    // color spectrum: black, blue, green, red
};
typedef struct graphhistory graphhistory_t;





/**
 * graph, name, data block size, pool
 */
int graphhistory_init (graphhistory_t*, char*, uint32_t, opool_t*);

/**
 * graph
 */
int graphhistory_destroy (graphhistory_t*);





/**
 *
 */
graph_t* graphhistory_graph (graphhistory_t*);

/**
 *
 */
int graphhistory_reset (graphhistory_t*);





/**
 *
 */
graphhistory_t* graphhistory_retain (graphhistory_t*);

/**
 *
 */
void graphhistory_release (graphhistory_t*);

#endif /* __GRAPH_HISTORY_H__ */
