/*
 *  sdrip.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.21.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __RFSPACE_SDRIP_H__
#define __RFSPACE_SDRIP_H__

#include <stdint.h>
#include "../device.h"

#define DEVICE_RFSPACE_SDRIP "RFSpace SDR-IP"

//
// sdrip
//
struct sdrip
{
	device_t device;									// parent class
	
};
typedef struct sdrip sdrip_t;

/**
 *
 */
sdrip_t* sdrip_alloc ();

/**
 *
 */
int sdrip_init (sdrip_t*);

/**
 *
 */
int sdrip_destroy (sdrip_t*);


// 8192 byte data block
// 

#endif /* __RFSPACE_SDRIP_H__ */
