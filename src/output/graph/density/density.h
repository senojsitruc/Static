/*
 *  density.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.24.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __GRAPH_DENSITY_H__
#define __GRAPH_DENSITY_H__

#include <stdint.h>
#include "../graph.h"
#include "../../../misc/mem/opool.h"

//
// graphdensity_data
//
struct graphdensity_data
{
	uint64_t *data;                      // graph data
};
typedef struct graphdensity_data graphdensity_data_t;

//
// graphdensity
//
struct graphdensity
{
	graph_t graph;                       // parent class
	graphdensity_data_t data;            // graph data
	uint64_t maxval;                     // current maximum value for any point on the graph
};
typedef struct graphdensity graphdensity_t;





/**
 * graph, name, data block size, pool
 */
int graphdensity_init (graphdensity_t*, char*, uint32_t, opool_t*);

/**
 * graph
 */
int graphdensity_destroy (graphdensity_t*);





/**
 *
 */
graph_t* graphdensity_graph (graphdensity_t*);





/**
 *
 */
graphdensity_t* graphdensity_retain (graphdensity_t*);

/**
 *
 */
void graphdensity_release (graphdensity_t*);

#endif /* __GRAPH_DENSITY_H__ */
