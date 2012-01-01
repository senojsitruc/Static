/*
 *  device.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "device.h"
#include "../driver/driver.h"
#include "../misc/logger.h"
#include <string.h>

void* __device_recv_thread (device_t*);
void* __device_send_thread (device_t*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
device_init (device_t *device, device_desc_t *desc)
{
	int error;
	
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (unlikely(0 != (error = connection_init(&device->connection))))
		LOG_ERROR_AND_RETURN(-101, "failed to connection_init, %d", error);
	
	memcpy(&device->desc, desc, sizeof(device_desc_t));
	device->samples = 0;
	
	if (unlikely(0 != (error = thread_init(&device->recv_thread, "device-read-thread", (thread_start_func)__device_recv_thread, device, NULL))))
		LOG_ERROR_AND_RETURN(-102, "failed to thread_init for recv_thread, %d", error);
	
	if (unlikely(0 != (error = thread_start(&device->recv_thread))))
		LOG_ERROR_AND_RETURN(-103, "failed to thread_start for recv_thread, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
device_destroy (device_t *device)
{
	int error;
	
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(0 != (error = connection_destroy(&device->connection))))
		LOG_ERROR_AND_RETURN(-101, "failed to connection_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
device_data_start (device_t *device)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(device->__data_beg == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'data_beg' callback");
	
	return (*device->__data_beg)(device);
}

/**
 *
 *
 */
int
device_data_stop (device_t *device)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(device->__data_end == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'data_end' callback");
	
	return (*device->__data_end)(device);
}

/**
 *
 *
 */
int
device_frequency_set (device_t *device, uint32_t frequency)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(device->__frequency_set == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'frequency_set' callback");
	
	return (*device->__frequency_set)(device, frequency);
}

/**
 *
 *
 */
int
device_frequency_get (device_t *device)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(device->__frequency_get == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'frequency_get' callback");
	
	return (*device->__frequency_get)(device);
}

/**
 *
 *
 */
int
device_gain_set (device_t *device, int32_t gain)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(device->__gain_set == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'gain_set' callback");
	
	return (*device->__gain_set)(device, gain);
}

/**
 *
 *
 */
int
device_span_set (device_t *device, uint32_t span)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(device->__span_set == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'span_set' callback");
	
	return (*device->__span_set)(device, span);
}

/**
 *
 */
int
device_program (device_t *device, chip_t *chip)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(chip == NULL))
		LOG_ERROR_AND_RETURN(-2, "null chip_t");
	
	if (unlikely(device->__program == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'program' callback");
	
	return (*device->__program)(device, chip);
}





#pragma mark -
#pragma mark other

/**
 *
 */
int
device_connect (device_t *device)
{
	int error;
	driver_t *driver;
	
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely((driver = device->desc.driver) == NULL))
		LOG_ERROR_AND_RETURN(-2, "null driver_t");
	
	if (unlikely(0 != (error = driver_connect(driver, &device->desc, &device->connection))))
		LOG_ERROR_AND_RETURN(-101, "failed to driver_connect, %d", error);
	
	if (device->__connect != NULL)
		if (unlikely(0 != (error = (*device->__connect)(device))))
			LOG_ERROR_AND_RETURN(-102, "failed to device->__connect, %d", error);
	
	return 0;
}

/**
 *
 */
int
device_disconnect (device_t *device)
{
	int error;
	driver_t *driver;
	
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely((driver = device->desc.driver) == NULL))
		LOG_ERROR_AND_RETURN(-2, "null driver_t");
	
	if (device->__disconnect != NULL)
		if (unlikely(0 != (error = (*device->__disconnect)(device))))
			LOG_ERROR_AND_RETURN(-101, "failed to device->__disconnect, %d", error);
	
	if (unlikely(0 != (error = driver_disconnect(driver, &device->desc))))
		LOG_ERROR_AND_RETURN(-102, "failed to driver_disconnect, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
device_connected (device_t *device)
{
	int error;
	driver_t *driver;
	
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely((driver = device->desc.driver) == NULL))
		LOG_ERROR_AND_RETURN(-2, "null driver_t");
	
	if (unlikely(0 > (error = driver_connected(driver, &device->desc))))
		LOG_ERROR_AND_RETURN(-101, "failed to driver_connected, %d", error);
	
	return 0;
}

/**
 *
 *
 */
connection_t*
device_connection (device_t *device)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null device_t");
	
	if (unlikely(device->connection.context1 == NULL))
		LOG_ERROR_AND_RETURN(NULL, "device not connected");
	
	return &device->connection;
}





#pragma mark -
#pragma mark data handlers

/**
 *
 *
 */
int
device_datastream_add (device_t *device, datastream_t *datastream)
{
	uint32_t i, count;
	
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(datastream == NULL))
		LOG_ERROR_AND_RETURN(-2, "null datastream_t");
	
	if (device->datastreamcnt == (count = sizeof(device->datastreams) / sizeof(void*)))
		LOG_ERROR_AND_RETURN(-101, "datastream list is full!");
	
	for (i = 0; i < count; ++i) {
		if (device->datastreams[i] == NULL) {
			device->datastreams[i] = datastream;
			device->datastreamcnt += 1;
			break;
		}
	}
	
	return 0;
}

/**
 *
 *
 */
int
device_datastream_del (device_t *device, datastream_t *datastream)
{
	uint32_t i, count;
	
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(datastream == NULL))
		LOG_ERROR_AND_RETURN(-2, "null datastream_t");
	
	count = sizeof(device->datastreams) / sizeof(void*);
	
	for (i = 0; i < count; ++i) {
		if (device->datastreams[i] == datastream) {
			device->datastreams[i] = NULL;
			device->datastreamcnt -= 1;
			LOG3("removed datastream");
			break;
		}
	}
	
	return 0;
}





#pragma mark -
#pragma mark datastream stuff

/**
 *
 *
 */
int
device_datastream_init (device_t *device, datastream_t *datastream)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	if (unlikely(device->__datastream_init == NULL))
		LOG_ERROR_AND_RETURN(-2, "null '__datastream_init' callback");
	
	return (*device->__datastream_init)(device, datastream);
}





#pragma mark -
#pragma mark thread

/**
 *
 *
 */
void*
__device_recv_thread (device_t *device)
{
	int error;
	
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null device_t");
	
	LOG3("[%s] starting", device->desc.name);
	
	while (!device->stop) {
		// don't do anything until we are connected to the device
		if (device->connection.context1 == NULL) {
			usleep(100000);
			continue;
		}
		
		if (unlikely(0 != (error = (*device->__message_read)(device))))
			LOG_ERROR_AND_GOTO(-101, done, "[%s] failed to device->__message_read, %d", device->desc.name, error);
	}
	
done:
	LOG3("[%s] stopping", device->desc.name);
	
	return NULL;
}
