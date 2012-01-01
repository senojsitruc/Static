/*
 *  driver.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "driver.h"
#include "../device/device.h"
#include "../misc/logger.h"

static int __driver_send (connection_t*, uint8_t*, uint32_t, uint32_t*);
static int __driver_recv (connection_t*, uint8_t*, uint32_t, uint32_t*);
static int __driver_available (connection_t*, uint32_t*);

#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
driver_init (driver_t *driver, char *name)
{
	int error;
	size_t name_l;
	
	if (unlikely(driver == NULL))
		LOG_ERROR_AND_RETURN(-1, "[%s] null driver_t", (name==NULL?"null":name));
	
	if (unlikely(name == NULL || (name_l = strlen(name)) == 0))
		LOG_ERROR_AND_RETURN(-2, "[null] null or zero-length name");
	
	if (unlikely(0 != (error = mutex_init(&driver->mutex, LOCK_TYPE_SPIN))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to mutex_init, %d", name, error);
	
	if (name_l >= sizeof(driver->desc.name))
		name_l = sizeof(driver->desc.name) - 1;
	
	memcpy(driver->desc.name, name, name_l);
	
	return 0;
}





#pragma mark -
#pragma mark other

/**
 *
 *
 */
int
driver_connect (driver_t *driver, device_desc_t *desc, connection_t *connection)
{
	int error;
	
	if (unlikely(driver == NULL))
		LOG_ERROR_AND_RETURN(-1, "null driver_t");
	
	if (unlikely(driver->connect == NULL))
		LOG_ERROR_AND_RETURN(-101, "[%s] null connect function pointer", driver->desc.name);
	
	if (unlikely(0 != (error = (*driver->connect)(driver, desc))))
		LOG_ERROR_AND_RETURN(-102, "[%s] failed to driver->connection, %d", driver->desc.name, error);
	
	connection->context1 = driver;
	connection->context2 = desc;
	connection->send = __driver_send;
	connection->recv = __driver_recv;
	connection->available = __driver_available;
	connection->endian = driver->endian;
	
	if (unlikely(0 != (error = connection_open(connection))))
		LOG_ERROR_AND_RETURN(-103, "[%s] failed to connection_open(0, %d", driver->desc.name, error);
	
	return 0;
}

/**
 *
 *
 */
int
driver_disconnect (driver_t *driver, device_desc_t *desc)
{
	int error;
	
	if (unlikely(driver == NULL))
		LOG_ERROR_AND_RETURN(-1, "null driver_t");
	
	if (unlikely(driver->disconnect == NULL))
		LOG_ERROR_AND_RETURN(-101, "[%s] null disconnect function pointer", driver->desc.name);
	
	if (unlikely(0 != (error = (*driver->disconnect)(driver, desc))))
		LOG_ERROR_AND_RETURN(-102, "[%s] failed to driver->disconnect, %d", driver->desc.name, error);
	
	return 0;
}

/**
 *
 *
 */
int
driver_connected (driver_t *driver, device_desc_t *desc)
{
	if (unlikely(driver == NULL))
		LOG_ERROR_AND_RETURN(-1, "null driver_t");
	
	if (unlikely(driver->connected == NULL))
		LOG_ERROR_AND_RETURN(-101, "[%s] null connected function pointer", driver->desc.name);
	
	return (*driver->connected)(driver, desc);
}

/**
 *
 *
 */
int
driver_findall (driver_t *driver, int (^callback)(device_desc_t*))
{
	if (unlikely(driver == NULL))
		LOG_ERROR_AND_RETURN(-1, "[null] null driver_t");
	
	if (unlikely(driver->findall == NULL))
		LOG_ERROR_AND_RETURN(-101, "[%s] null findall function pointer", driver->desc.name);
	
	return (*driver->findall)(driver, callback);
}





#pragma mark -
#pragma mark connection callbacks

/**
 *
 *
 */
static int
__driver_send (connection_t *connection, uint8_t *data, uint32_t data_i, uint32_t *data_o)
{
	driver_t *driver;
	device_desc_t *desc;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	driver = (driver_t*)connection->context1;
	desc = (device_desc_t*)connection->context2;
	
	if (unlikely(driver->send == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'driver->send' callback");
	
	return (*driver->send)(driver, desc, data, data_i, data_o);
}

/**
 *
 *
 */
static int
__driver_recv (connection_t *connection, uint8_t *data, uint32_t data_i, uint32_t *data_o)
{
	driver_t *driver;
	device_desc_t *desc;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	driver = (driver_t*)connection->context1;
	desc = (device_desc_t*)connection->context2;
	
	if (unlikely(driver->recv == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'driver->recv' callback");
	
	return (*driver->recv)(driver, desc, data, data_i, data_o);
}

/**
 *
 *
 */
static int
__driver_available (connection_t *connection, uint32_t *data_o)
{
	driver_t *driver;
	device_desc_t *desc;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	driver = (driver_t*)connection->context1;
	desc = (device_desc_t*)connection->context2;
	
	if (unlikely(driver->available == NULL))
		LOG_ERROR_AND_RETURN(-2, "null 'driver->available' callback");
	
	return (*driver->available)(driver, desc, data_o);
}
