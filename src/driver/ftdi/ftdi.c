/*
 *  ftdi.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.12.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ftdi.h"
#include "../../device/device.h"
#include "../../extern/ftdi/ftdi.h"
#include "../../misc/dump.h"
#include "../../misc/logger.h"
#include "../../misc/net/connection.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
ftdi_alloc (ftdi_t **ftdi)
{
	int error;
	
	if (unlikely(ftdi == NULL))
		LOG_ERROR_AND_RETURN(-1, "null return pointer");
	
	*ftdi = NULL;
	
	if (unlikely(NULL == (*ftdi = (ftdi_t*)malloc( sizeof(ftdi_t) ))))
		LOG_ERROR_AND_RETURN(-101, "failed to malloc, %s", strerror(errno));
	
	memset(*ftdi, 0, sizeof(ftdi_t));
	
	if (unlikely(0 != (error = ftdi_xinit(*ftdi))))
		LOG_ERROR_AND_RETURN(-102, "failed to ftdi_init, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
ftdi_xinit (ftdi_t *ftdi)
{
	int error;
	
	if (unlikely(ftdi == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftdi_t");
	
	if (unlikely(0 != (error = driver_init((driver_t*)ftdi, DRIVER_FTDI))))
		LOG_ERROR_AND_RETURN(-101, "failed to driver_init, %d", error);
	
	ftdi->driver.connect = (driver_connect_func)ftdi_connect;
	ftdi->driver.disconnect = (driver_disconnect_func)ftdi_disconnect;
	ftdi->driver.connected = (driver_connected_func)ftdi_connected;
	ftdi->driver.send = (driver_send_func)ftdi_send;
	ftdi->driver.recv = (driver_recv_func)ftdi_recv;
	ftdi->driver.available = (driver_available_func)ftdi_available;
	ftdi->driver.findall = (driver_findall_func)ftdi_findall;
	ftdi->driver.endian = CONNECTION_LITTLE_ENDIAN;
	
	if (0 != (error = ftdi_init(&ftdi->ftdic)))
		LOG_ERROR_AND_RETURN(-102, "failed to ftdi_init(), %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
ftdi_connect (ftdi_t *ftdi, device_desc_t *desc)
{
	int error = 0;
	
	if (unlikely(ftdi == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftdi_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (unlikely(desc->handle != NULL))
		LOG_ERROR_AND_RETURN(-3, "already connected");
	
	if (0 == strlen(desc->name))
		LOG_ERROR_AND_RETURN(-101, "no device name to open");
	
	mutex_lock(&ftdi->driver.mutex);
	
	ftdi->stop = 0;
	
	// we'll take any interface available
	if (0 != (error = ftdi_set_interface(&ftdi->ftdic, INTERFACE_ANY)))
		LOG_ERROR_AND_GOTO(-102, done, " [%s] failed to ftdi_set_interface(), %d", desc->name, error);
	
	// open a connection to the device
	if (0 != (error = ftdi_usb_open(&ftdi->ftdic, 0x0403, 0x6001)))
		LOG_ERROR_AND_GOTO(-103, done, " [%s] failed to ftdi_usb_open(), %d", desc->name, error);
	
	// reset the usb device
	if (0 != (error = ftdi_usb_reset(&ftdi->ftdic)))
		LOG_ERROR_AND_GOTO(-104, done, " [%s] failed to ftdi_usb_reset(), %d", desc->name, error);
	
	// what's a good baudrate to use?
	if (0 != (error = ftdi_set_baudrate(&ftdi->ftdic, 921600)))
		LOG_ERROR_AND_GOTO(-105, done, " [%s] failed to ftdi_set_baudrate(), %d", desc->name, error);
	
	/*
	if (0 != (error = ftdi_set_latency_timer(&ftdi->ftdic, 2)))
		LOG_ERROR_AND_GOTO(-106, done, " [%s] failed to ftdi_set_latency_timer(), %d", desc->name, error);
	*/
	
	// set the read buffer chunk size
	if (0 != (error = ftdi_read_data_set_chunksize(&ftdi->ftdic, 16384)))
		LOG_ERROR_AND_GOTO(-107, done, " [%s] failed to ftdi_read_data_set_chunksize(), %d", desc->name, error);
	
	// set the write chunk size
	if (0 != (error = ftdi_write_data_set_chunksize(&ftdi->ftdic, 16384)))
		LOG_ERROR_AND_GOTO(-108, done, " [%s] failed to ftdi_write_data_set_chunksize(), %d", desc->name, error);
	
	if (0 != (error = ftdi_usb_purge_buffers(&ftdi->ftdic)))
		LOG_ERROR_AND_GOTO(-109, done, " [%s] failed to ftdi_usb_purge_buffers(), %d", desc->name, error);
	
	// this'll be our "handle" for the device connection
	desc->handle = ftdi;
	
done:
	mutex_unlock(&ftdi->driver.mutex);
	return error;
}

/**
 *
 *
 */
int
ftdi_disconnect (ftdi_t *ftdi, device_desc_t *desc)
{
	int error = 0;
	
	if (unlikely(ftdi == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftdi_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (unlikely(desc->handle == NULL))
		LOG_ERROR_AND_RETURN(-3, "not connected");
	
	ftdi->stop = 1;
	
	mutex_lock(&ftdi->driver.mutex);
	
	// purge the rx and tx buffers
	if (0 != (error = ftdi_usb_purge_buffers(&ftdi->ftdic)))
		LOG_ERROR_AND_GOTO(-101, done, " [%s] failed to ftdi_usb_purge_buffers(), %d", desc->name, error);
	
	// close the device connection
	if (0 != (error = ftdi_usb_close(&ftdi->ftdic)))
		LOG_ERROR_AND_GOTO(-102, done, " [%s] failed to ftdi_usb_close(), %d", desc->name, error);
	
	desc->handle = NULL;
	
done:
	mutex_unlock(&ftdi->driver.mutex);
	return error;
}

/**
 *
 *
 */
int
ftdi_connected (ftdi_t *ftdi, device_desc_t *desc)
{
	if (unlikely(ftdi == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftdi_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	return desc->handle != NULL;
}

/**
 *
 *
 */
int
ftdi_send (ftdi_t *ftdi, device_desc_t *desc, uint8_t *data, uint32_t data_i, uint32_t *data_o)
{
	int error=0, bytes;
	
	if (unlikely(ftdi == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftdi_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	if (unlikely(data_i == 0))
		LOG_ERROR_AND_RETURN(-4, "invalid data length (0)");
	
	if (unlikely(data_o == NULL))
		LOG_ERROR_AND_RETURN(-5, "null data return pointer");
	
	if (unlikely(desc->handle == NULL))
		LOG_ERROR_AND_RETURN(-6, "not connected");
	
	//hexdump(data, data_i);
	
	mutex_lock(&ftdi->driver.mutex);
	
	if (unlikely(0 > (bytes = ftdi_write_data(&ftdi->ftdic, data, (int)data_i))))
		LOG_ERROR_AND_GOTO(-101, done, " [%s] failed to ftdi_write_data(), %d", desc->name, bytes);
	
	*data_o = (uint32_t)bytes;
	
done:
	mutex_unlock(&ftdi->driver.mutex);
	return error;
}

/**
 *
 *
 */
int
ftdi_recv (ftdi_t *ftdi, device_desc_t *desc, uint8_t *data, uint32_t data_i, uint32_t *data_o)
{
	int bytes, error=0;
	
	if (unlikely(ftdi == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftdi_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	if (unlikely(data_i == 0))
		LOG_ERROR_AND_RETURN(-4, "invalid data length (0)");
	
	if (unlikely(data_o == NULL))
		LOG_ERROR_AND_RETURN(-5, "null data return pointer");
	
	if (unlikely(desc->handle == NULL))
		LOG_ERROR_AND_RETURN(-6, "not connected");
	
	mutex_lock(&ftdi->driver.mutex);
	
	if (unlikely(0 > (bytes = ftdi_read_data(&ftdi->ftdic, data, (int)data_i))))
		LOG_ERROR_AND_GOTO(-101, done, " [%s] failed to ftdi_read_data(), %d", desc->name, bytes);
	
	*data_o = (uint32_t)bytes;
	
done:
	mutex_unlock(&ftdi->driver.mutex);
	return error;
}

/**
 *
 *
 */
int
ftdi_available (ftdi_t *ftdi, device_desc_t *desc, uint32_t *data_i)
{
	int error = 0;
	
	if (unlikely(ftdi == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftdi_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (unlikely(data_i == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data length");
	
	if (unlikely(desc->handle == NULL))
		LOG_ERROR_AND_RETURN(-4, "not connected");
	
	mutex_lock(&ftdi->driver.mutex);
	
	/*
	if (ftdi->ftdic.readbuffer_remaining == 0)
		ftdi_read_refill_buffer(&ftdi->ftdic);
	*/
	
	*data_i = ftdi->ftdic.readbuffer_remaining;
	
//done:
	mutex_unlock(&ftdi->driver.mutex);
	return error;
}





#pragma mark -
#pragma mark other

/**
 *
 *
 */
int
ftdi_findall (ftdi_t *ftdi, int (^callback)(struct device_desc*))
{
	int error = 0;
	char manu[128], desc[128], numb[128];
	struct ftdi_device_list *devs=NULL, *dev;
	device_desc_t device_desc;
	
	if (unlikely(ftdi == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftdi_t");
	
	if (unlikely(callback == NULL))
		LOG_ERROR_AND_RETURN(-2, "null callback");
	
	if (0 > (error = ftdi_usb_find_all(&ftdi->ftdic, &devs, 0x0403, 0x6001)))
		LOG_ERROR_AND_RETURN(-101, "failed to ftdi_usb_find_all(), %d", error);
	
	LOG3("found %d ftdi device(s)", error);
	
	if (error == 0)
		return 0;
	
	for (dev = devs; dev != NULL;) {
		memset(&device_desc, 0, sizeof(device_desc_t));
		
		/*
		device_desc.flags = devlist[i].Flags;
		device_desc.type = devlist[i].Type;
		device_desc.id = devlist[i].ID;
		device_desc.location = devlist[i].LocId;
		*/
		device_desc.driver = (driver_t*)ftdi;
//	device_desc.vendor = dev->dev->descriptor.idVendor;
//	device_desc.product = dev->dev->descriptor.idProduct;
		
		if (0 > (error = ftdi_usb_get_strings(&ftdi->ftdic, dev->dev, manu, sizeof(manu), desc, sizeof(desc), numb, sizeof(numb))))
			LOG_ERROR_AND_GOTO(-102, done, "failed to ftdi_usb_get_strings(), %d", error);
		
		LOG3("manu=%s, desc=%s, numb=%s", manu, desc, numb);
		
		memcpy(device_desc.serial, numb, strlen(numb));
		memcpy(device_desc.name, desc, strlen(desc));
		
		if (unlikely(0 != (error = callback(&device_desc))))
			LOG_ERROR_AND_GOTO(-103, done, "failed to callback for '%s', %d", device_desc.name, error);
		
		dev = dev->next;
	}
	
done:
	ftdi_list_free(&devs);
	
	return error;
}
