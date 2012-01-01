/*
 *  driver.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <stdint.h>
#include "../misc/net/connection.h"
#include "../misc/thread/mutex.h"

struct driver;
struct device_desc;

//
// driver_desc
//
struct driver_desc
{
	char name[50];										// driver name
	uint8_t enabled;									// enabled
	
	int (*alloc_func)(struct driver**);
};
typedef struct driver_desc driver_desc_t;

//
// driver
//
struct driver
{
	driver_desc_t desc;								// driver description
	connection_endian endian;					// driver endianness
	mutex_t mutex;										// protected read/write
	
	int (*connect)(struct driver*, struct device_desc*);
	int (*disconnect)(struct driver*, struct device_desc*);
	int (*connected)(struct driver*, struct device_desc*);
	int (*send)(struct driver*, struct device_desc*, uint8_t*, uint32_t, uint32_t*);
	int (*recv)(struct driver*, struct device_desc*, uint8_t*, uint32_t, uint32_t*);
	int (*available)(struct driver*, struct device_desc*, uint32_t*);
	int (*findall)(struct driver*, int (^)(struct device_desc*));
};
typedef struct driver driver_t;

//
// driver_alloc_func
//
typedef int (*driver_alloc_func)(driver_t**);

//
// driver_connect_func
//
typedef int (*driver_connect_func)(driver_t*, struct device_desc*);

//
// driver_disconnect_func
//
typedef int (*driver_disconnect_func)(driver_t*, struct device_desc*);

//
// driver_connected_func
//
typedef int (*driver_connected_func)(driver_t*, struct device_desc*);

//
// driver_send_func
//
typedef int (*driver_send_func)(driver_t*, struct device_desc*, uint8_t*, uint32_t, uint32_t*);

//
// driver_recv_func
//
typedef int (*driver_recv_func)(driver_t*, struct device_desc*, uint8_t*, uint32_t, uint32_t*);

//
// driver_available_func
//
typedef int (*driver_available_func)(driver_t*, struct device_desc*, uint32_t*);

//
// driver_findall_func
//
typedef int (*driver_findall_func)(driver_t*, int (^)(struct device_desc*));

/**
 * driver, name
 */
int driver_init (driver_t*, char*);

/**
 *
 */
int driver_connect (driver_t*, struct device_desc*, connection_t*);

/**
 *
 */
int driver_disconnect (driver_t*, struct device_desc*);

/**
 *
 */
int driver_connected (driver_t*, struct device_desc*);

/**
 *
 */
int driver_findall (driver_t*, int (^)(struct device_desc*));

#endif /* __DRIVER_H__ */
