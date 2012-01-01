/*
 *  ftd2xx.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __DRIVER_FTD2XX_H__
#define __DRIVER_FTD2XX_H__

#include "../driver.h"
#include "../../device/device.h"
#include "../../extern/ftd2xx/ftd2xx.h"
#include "../../extern/ftd2xx/WinTypes.h"

#define DRIVER_FTD2XX "FTDI2xx"

struct device_desc;

//
// ftd2xx
//
struct ftd2xx
{
	driver_t driver;									// parent class
};
typedef struct ftd2xx ftd2xx_t;

/**
 *
 */
int ftd2xx_alloc (ftd2xx_t**);

/**
 *
 */
int ftd2xx_init (ftd2xx_t*);

/**
 * Close the handle if it's open.
 */
int ftd2xx_destroy (ftd2xx_t*);

/**
 *
 */
int ftd2xx_connect (ftd2xx_t*, device_desc_t*);

/**
 *
 */
int ftd2xx_disconnect (ftd2xx_t*, device_desc_t*);

/**
 *
 */
int ftd2xx_connected (ftd2xx_t*, device_desc_t*);

/**
 *
 */
int ftd2xx_send (ftd2xx_t*, device_desc_t*, uint8_t*, uint32_t, uint32_t*);

/**
 *
 */
int ftd2xx_recv (ftd2xx_t*, device_desc_t*, uint8_t*, uint32_t, uint32_t*);

/**
 *
 */
int ftd2xx_available (ftd2xx_t*, device_desc_t*, uint32_t*);

/**
 *
 */
int ftd2xx_findall (ftd2xx_t*, int (^)(struct device_desc*));




/**
 *
 */
int ftd2xx_read (ftd2xx_t*, uint8_t*, uint32_t);


// get device list
// open specific device
// write data
// read data

// driver version
// library version

// reset device
// reset port

#endif /* __DRIVER_FTD2XX_H__ */
