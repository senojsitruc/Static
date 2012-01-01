/*
 *  ftdi.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.12.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __DRIVER_FTDI_H__
#define __DRIVER_FTDI_H__

#include "../driver.h"
#include "../../device/device.h"
#include "../../extern/ftdi/ftdi.h"

#define DRIVER_FTDI "FTDI"

struct device_desc;

//
// ftdi
//
struct ftdi
{
	driver_t driver;                     // parent class
	struct ftdi_context ftdic;           // ftdi context
	int stop;                            // time to stop
};
typedef struct ftdi ftdi_t;

/**
 *
 */
int ftdi_alloc (ftdi_t**);

/**
 * xinit, because ftdi.h uses init.
 */
int ftdi_xinit (ftdi_t*);

/**
 * Close the handle if it's open.
 */
int ftdi_destroy (ftdi_t*);

/**
 *
 */
int ftdi_connect (ftdi_t*, device_desc_t*);

/**
 *
 */
int ftdi_disconnect (ftdi_t*, device_desc_t*);

/**
 *
 */
int ftdi_connected (ftdi_t*, device_desc_t*);

/**
 *
 */
int ftdi_send (ftdi_t*, device_desc_t*, uint8_t*, uint32_t, uint32_t*);

/**
 *
 */
int ftdi_recv (ftdi_t*, device_desc_t*, uint8_t*, uint32_t, uint32_t*);

/**
 *
 */
int ftdi_available (ftdi_t*, device_desc_t*, uint32_t*);

/**
 *
 */
int ftdi_findall (ftdi_t*, int (^)(struct device_desc*));

/**
 *
 */
int ftdi_read (ftdi_t*, uint8_t*, uint32_t);

#endif /* __DRIVER_FTDI_H__ */
