/*
 *  device.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>
#include "../chip/chip.h"
#include "../output/datastream.h"
#include "../dsp/dsp.h"
#include "../misc/net/connection.h"
#include "../misc/thread/thread.h"





struct device;
struct driver;

//
// device_desc
//
struct device_desc
{
	uint32_t flags;										   // 
	uint32_t type;										   // 
	uint32_t id;											   // 
	uint32_t location;								   // 
	char serial[50];									   // 
	char name[50];										   // 
	uint8_t enabled;									   // 
	struct driver *driver;						   // 
	void *handle;											   // 
	
	int (*alloc)(struct device**, struct device_desc*);
	int (*isme)(struct device_desc*);
};
typedef struct device_desc device_desc_t;

//
// device
//
struct device
{
	/* connection related */
	device_desc_t desc;								// device description
	connection_t connection;					// device connection
	thread1_t recv_thread;						// message receive thread
	thread1_t send_thread;						// message send thread
	uint32_t stop;										// thread stop flag
	
	/* device status */
	double decimation;								// decimation rate
	double samplerate;								// sample rate
	uint32_t frequency;								// frequency
	int8_t gain;											// rf gain
	uint32_t span;										// frequency span
	uint32_t data_on;									// receiving data (1 or 0)
	uint64_t samples;									// number of samples received
	
	/* data handlers */
	uint32_t datastreamcnt;              // number of data stream objects
	datastream_t* datastreams[50];       // data stream objects
	
	/* derived class overloads */
	int (*__connect)(struct device*);
	int (*__disconnect)(struct device*);
	int (*__message_read)(struct device*);
	int (*__frequency_set)(struct device*, uint32_t);
	int (*__frequency_get)(struct device*);
	int (*__gain_set)(struct device*, int32_t);
	int (*__gain_get)(struct device*);
	int (*__span_set)(struct device*, uint32_t);
	int (*__span_get)(struct device*);
	int (*__data_beg)(struct device*);
	int (*__data_end)(struct device*);
	int (*__program)(struct device*, chip_t*);
	int (*__datastream_init)(struct device*, datastream_t*);
};
typedef struct device device_t;





//
// device_alloc_func
//
typedef int (*device_alloc_func)(device_t**, device_desc_t*);

//
// device_desc_func
//
typedef int (*device_isme_func)(device_desc_t*);

//
// device_connect_func
//
typedef int (*device_connect_func)(device_t*);

//
// device_disconnect_func
//
typedef int (*device_disconnect_func)(device_t*);

//
// message_read_func
//
typedef int (*device_message_read_func)(device_t*);

//
// device_data_beg_func
//
typedef int (*device_data_beg_func)(device_t*);

//
// device_data_end_func
//
typedef int (*device_data_end_func)(device_t*);

//
// device_frequency_set_func
//
typedef int (*device_frequency_set_func)(device_t*, uint32_t);

//
// device_frequency_get_func
//
typedef int (*device_frequency_get_func)(device_t*);

//
// device_gain_set_func
//
typedef int (*device_gain_set_func)(device_t*, int32_t);

//
// device_program_func
//
typedef int (*device_program_func)(device_t*, chip_t*);

//
// device_span_set_func
//
typedef int (*device_span_set_func)(device_t*, uint32_t);

//
// device_span_get_func
//
typedef int (*device_span_get_func)(device_t*);

//
// device_datastream_init_func
//
typedef int (*device_datastream_init_func)(device_t*, datastream_t*);





/**
 *
 */
int device_init (device_t*, device_desc_t*);

/**
 *
 */
int device_destroy (device_t*);





/**
 *
 */
int device_connect (device_t*);

/**
 *
 */
int device_disconnect (device_t*);

/**
 *
 */
int device_connected (device_t*);

/**
 *
 */
connection_t* device_connection (device_t*);





/**
 *
 */
int device_data_start (device_t*);

/**
 *
 */
int device_data_stop (device_t*);

/**
 *
 */
int device_frequency_set (device_t*, uint32_t);

/**
 *
 */
int device_frequency_get (device_t*);

/**
 *
 */
int device_gain_set (device_t*, int32_t);

/**
 *
 */
int device_span_set (device_t*, uint32_t);

/**
 *
 */
int device_program (device_t*, chip_t*);





/**
 *
 */
int device_datastream_add (device_t*, datastream_t*);

/**
 *
 */
int device_datastream_del (device_t*, datastream_t*);





/**
 *
 */
int device_datastream_init (device_t*, datastream_t*);

#endif /* __DEVICE_H__ */
