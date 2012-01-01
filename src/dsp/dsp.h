/*
 *  dsp.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __DSP_H__
#define __DSP_H__

#include <stdint.h>
#include "../misc/mem/cobject.h"
#include "../misc/mem/opool.h"

struct device;

//
// dsp_state
//
typedef enum
{
	DSP_STATE_ENABLED  = (1 << 0),       // enabled
	DSP_STATE_DISABLED = (1 << 1)        // disabled
} dsp_state;

//
// dsp_datatype
//
typedef enum
{
	DSP_DATA_TYPE_SINT8  = (1 << 0),     // sint8
	DSP_DATA_TYPE_UINT8  = (1 << 1),     // uint8
	DSP_DATA_TYPE_SINT16 = (1 << 2),     // sint16
	DSP_DATA_TYPE_UINT16 = (1 << 3),     // uint16
	DSP_DATA_TYPE_SINT32 = (1 << 4),     // sint32
	DSP_DATA_TYPE_UINT32 = (1 << 5),     // uint32
	DSP_DATA_TYPE_SINT64 = (1 << 6),     // sint64
	DSP_DATA_TYPE_UINT64 = (1 << 7),     // uint64
	DSP_DATA_TYPE_FLOAT  = (1 << 8),     // float
	DSP_DATA_TYPE_DOUBLE = (1 << 9)      // double
} dsp_datatype;

//
// dsp
//
struct dsp
{
	cobject_t cobject;                   // parent class
	
	struct device *device;               // device
	dsp_state state;                     // enabled, disabled, etc
	
	/* derived class overloads */
	int (*__feed)(struct dsp*, uint32_t*, void*);
	int (*__reset)(struct dsp*);
};
typedef struct dsp dsp_t;





//
// function typedefs
//
typedef int (*__dsp_feed_func)(dsp_t*, uint32_t*, void*);
typedef int (*__dsp_reset_func)(dsp_t*);





/**
 * dsp, name, destroy function pointer, pool
 */
int dsp_init (dsp_t*, char*, cobject_destroy_func, opool_t*);

/**
 *
 */
int dsp_destroy (dsp_t*);





/**
 * Feed some data to the dsp. The size indicates the size of the data pointer in _bytes_; not the
 * number of values of whatever type (double, short, etc). Upon completion of processing, a dsp
 * should update the size pointer if it has changed the amount of available data.
 */
int dsp_feed (dsp_t*, uint32_t*, void*);

/**
 * Tell the dsp to reset its state.
 */
int dsp_reset (dsp_t*);





/**
 *
 */
dsp_t* dsp_retain (dsp_t*);

/**
 *
 */
void dsp_release (dsp_t*);

#endif /* __DSP_H__ */
