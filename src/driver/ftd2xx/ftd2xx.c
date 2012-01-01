/*
 *  ftd2xx.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "ftd2xx.h"
#include "../../device/device.h"
#include "../../extern/ftd2xx/ftd2xx.h"
#include "../../extern/ftd2xx/WinTypes.h"
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
ftd2xx_alloc (ftd2xx_t **ftd2xx)
{
	int error;
	
	if (unlikely(ftd2xx == NULL))
		LOG_ERROR_AND_RETURN(-1, "null return pointer");
	
	*ftd2xx = NULL;
	
	if (unlikely(NULL == (*ftd2xx = (ftd2xx_t*)malloc( sizeof(ftd2xx_t) ))))
		LOG_ERROR_AND_RETURN(-101, "failed to malloc, %s", strerror(errno));
	
	memset(*ftd2xx, 0, sizeof(ftd2xx_t));
	
	if (unlikely(0 != (error = ftd2xx_init(*ftd2xx))))
		LOG_ERROR_AND_RETURN(-102, "failed to ftd2xx_init, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
ftd2xx_init (ftd2xx_t *ftd2xx)
{
	int error;
	
	if (unlikely(ftd2xx == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftd2xx_t");
	
	if (unlikely(0 != (error = driver_init((driver_t*)ftd2xx, DRIVER_FTD2XX))))
		LOG_ERROR_AND_RETURN(-101, "failed to driver_init, %d", error);
	
	ftd2xx->driver.connect = (driver_connect_func)ftd2xx_connect;
	ftd2xx->driver.disconnect = (driver_disconnect_func)ftd2xx_disconnect;
	ftd2xx->driver.connected = (driver_connected_func)ftd2xx_connected;
	ftd2xx->driver.send = (driver_send_func)ftd2xx_send;
	ftd2xx->driver.recv = (driver_recv_func)ftd2xx_recv;
	ftd2xx->driver.available = (driver_available_func)ftd2xx_available;
	ftd2xx->driver.findall = (driver_findall_func)ftd2xx_findall;
	ftd2xx->driver.endian = CONNECTION_LITTLE_ENDIAN;
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
ftd2xx_connect (ftd2xx_t *ftd2xx, device_desc_t *desc)
{
	int error = 0;
	FT_STATUS ftstatus;
	
	if (ftd2xx == NULL)
		LOG_ERROR_AND_RETURN(-1, "null ftd2xx_t");
	
	if (desc == NULL)
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (desc->handle != NULL)
		LOG_ERROR_AND_RETURN(-3, "already connected");
	
	if (0 == strlen(desc->name))
		LOG_ERROR_AND_RETURN(-101, "no device name to open");
	
	mutex_lock(&ftd2xx->driver.mutex);
	
	// open by description
	if (FT_OK != (ftstatus = FT_OpenEx(desc->name, FT_OPEN_BY_DESCRIPTION, (FT_HANDLE*)&desc->handle)))
		LOG_ERROR_AND_GOTO(-102, done, " [%s] failed to FT_OpenEx(), %lu", desc->name, ftstatus);
	
	/*
	if (FT_OK != (ftstatus = FT_SetTimeouts((FT_HANDLE)desc->handle, 1000, 1000)))
		LOG_ERROR_AND_GOTO(-103, done, " [%s] failed to FT_SetTimeouts(), %lu", desc->name, ftstatus);
	*/
	
	FT_ResetDevice((FT_HANDLE)desc->handle);
	FT_Purge((FT_HANDLE)desc->handle, FT_PURGE_RX | FT_PURGE_TX);
	
done:
	mutex_unlock(&ftd2xx->driver.mutex);
	return error;
}

/**
 *
 *
 */
int
ftd2xx_disconnect (ftd2xx_t *ftd2xx, device_desc_t *desc)
{
	int error = 0;
	FT_STATUS ftstatus;
	
	if (ftd2xx == NULL)
		LOG_ERROR_AND_RETURN(-1, "null ftd2xx_t");
	
	if (desc == NULL)
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (desc->handle == NULL)
		LOG_ERROR_AND_RETURN(-3, "not connected");
	
	mutex_lock(&ftd2xx->driver.mutex);
	
	// reset the device
	FT_ResetDevice((FT_HANDLE)desc->handle);
	FT_Purge((FT_HANDLE)desc->handle, FT_PURGE_RX | FT_PURGE_TX);
	
	// close the device handle
	if (FT_OK != (ftstatus = FT_Close((FT_HANDLE)desc->handle)))
		LOG_ERROR_AND_GOTO(-101, done, "failed to FT_Close, %lu", ftstatus);
	
	desc->handle = NULL;
	
done:
	mutex_unlock(&ftd2xx->driver.mutex);
	return error;
}

/**
 *
 *
 */
int
ftd2xx_connected (ftd2xx_t *ftd2xx, device_desc_t *desc)
{
	if (unlikely(ftd2xx == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftd2xx_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	return desc->handle != NULL;
}

/**
 *
 *
 */
int
ftd2xx_send (ftd2xx_t *ftd2xx, device_desc_t *desc, uint8_t *data, uint32_t data_i, uint32_t *data_o)
{
	int error = 0;
	FT_STATUS ftstatus;
	DWORD bytes = 0;
	
	if (unlikely(ftd2xx == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftd2xx_t");
	
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
	
	hexdump(data, (int)data_i);
	
	mutex_lock(&ftd2xx->driver.mutex);
	
	if (unlikely(FT_OK != (ftstatus = FT_Write((FT_HANDLE)desc->handle, data, data_i, &bytes))))
		LOG_ERROR_AND_GOTO(-101, done, "failed to FT_Write, %lu", ftstatus);
	
	*data_o = (uint32_t)bytes;
	
done:
	mutex_unlock(&ftd2xx->driver.mutex);
	return error;
}

/**
 *
 *
 */
int
ftd2xx_recv (ftd2xx_t *ftd2xx, device_desc_t *desc, uint8_t *data, uint32_t data_i, uint32_t *data_o)
{
	int error = 0;
	FT_STATUS ftstatus;
	DWORD bytes;
	
	if (unlikely(ftd2xx == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftd2xx_t");
	
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
	
	mutex_lock(&ftd2xx->driver.mutex);
	
	if (unlikely(FT_OK != (ftstatus = FT_Read((FT_HANDLE)desc->handle, data, data_i, &bytes))))
		LOG_ERROR_AND_GOTO(-101, done, "failed to FT_Read, %lu", ftstatus);
	
//if (bytes != 0) {
//	LOG3("  read %d bytes", (int)bytes);
//	hexdump(data, bytes > 50 ? 50 : bytes);
//}
	
	*data_o = (uint32_t)bytes;
	
done:
	mutex_unlock(&ftd2xx->driver.mutex);
	return error;
}

/**
 *
 *
 */
int
ftd2xx_available (ftd2xx_t *ftd2xx, device_desc_t *desc, uint32_t *data_i)
{
	int error = 0;
	FT_STATUS ftstatus;
	DWORD bytes = 0;
	
	if (unlikely(ftd2xx == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftd2xx_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (unlikely(data_i == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data length");
	
	if (unlikely(desc->handle == NULL))
		LOG_ERROR_AND_RETURN(-4, "not connected");
	
	mutex_lock(&ftd2xx->driver.mutex);
	
	if (unlikely(FT_OK != (ftstatus = FT_GetQueueStatus((FT_HANDLE)desc->handle, &bytes))))
		LOG_ERROR_AND_GOTO(-101, done, "failed to FT_GetQueueStatus, %lu", ftstatus);
	
	*data_i = (uint32_t)bytes;
	
done:
	mutex_unlock(&ftd2xx->driver.mutex);
	return error;
}





#pragma mark -
#pragma mark other

/**
 *
 *
 */
int
ftd2xx_findall (ftd2xx_t *ftd2xx, int (^callback)(struct device_desc*))
{
	int error = 0;
	FT_STATUS ftstatus;
//FT_HANDLE fthandle;
	DWORD devcount=0;
	FT_DEVICE_LIST_INFO_NODE *devlist = NULL;
	device_desc_t device_desc;
	
	if (unlikely(ftd2xx == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ftd2xx_t");
	
	if (unlikely(callback == NULL))
		LOG_ERROR_AND_RETURN(-2, "null callback");
	
	// find out how many devices are known
	if (FT_OK != (ftstatus = FT_CreateDeviceInfoList(&devcount)))
		LOG_ERROR_AND_RETURN(-101, "failed to FT_CreateDeviceInfoList, %lu", ftstatus);
	
	LOG3("found %lu ftd2xx device(s)", devcount);
	
	// stop now if we can't find any devices
	if (0 == devcount)
		return 0;
	
	// allocate a device list large enough to contain all of the known devices
	if (NULL == (devlist = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * devcount)))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc(%lu), %s", (sizeof(FT_DEVICE_LIST_INFO_NODE) * devcount), strerror(errno));
	
	// zero out the device list
	memset(devlist, 0, sizeof(FT_DEVICE_LIST_INFO_NODE) * devcount);
	
	// get device information for all of the known devices
	if (FT_OK != (ftstatus = FT_GetDeviceInfoList(devlist, &devcount)))
		LOG_ERROR_AND_GOTO(-103, done, "failed to FT_GetDeviceInfoList, %lu", ftstatus);
	
	// print out the device information for all of the known devices and call the specified callback
	for (DWORD i = 0; i < devcount; ++i) {
		LOG3("ftd2xx device[%lu] = {", i);
		LOG3("  flags = 0x%08X", (uint32_t)devlist[i].Flags);
		LOG3("  type = 0x%08X", (uint32_t)devlist[i].Type);
		LOG3("  id = 0x%08X", (uint32_t)devlist[i].ID);
		LOG3("  locid = 0x%08X", (uint32_t)devlist[i].LocId);
		LOG3("  serial = %s", devlist[i].SerialNumber);
		LOG3("  description = %s", devlist[i].Description);
		LOG3("}");
		
		memset(&device_desc, 0, sizeof(device_desc_t));
		device_desc.flags = (uint32_t)devlist[i].Flags;
		device_desc.type = (uint32_t)devlist[i].Type;
		device_desc.id = (uint32_t)devlist[i].ID;
		device_desc.location = (uint32_t)devlist[i].LocId;
		device_desc.driver = (driver_t*)ftd2xx;
		memcpy(device_desc.serial, devlist[i].SerialNumber, strlen(devlist[i].SerialNumber));
		memcpy(device_desc.name, devlist[i].Description, strlen(devlist[i].Description));
		
		if (unlikely(0 != (error = callback(&device_desc))))
			LOG_ERROR_AND_CONTINUE("failed to callback for '%s', %d", device_desc.name, error);
	}
	
done:
	/*
	if (devlist != NULL)
		free(devlist);
	*/
	
	return error;
}
