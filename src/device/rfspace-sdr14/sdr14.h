/*
 *  sdr14.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.21.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __RFSPACE_SDR14_H__
#define __RFSPACE_SDR14_H__

#include <stdint.h>
#include "../device.h"

#define DEVICE_RFSPACE_SDR14 "RFSpace SDR-14"

//
// sdr14
//
struct sdr14
{
	device_t device;									// parent class
	
};
typedef struct sdr14 sdr14_t;

/**
 *
 */
sdr14_t* sdr14_alloc ();

/**
 *
 */
int sdr14_init (sdr14_t*);

/**
 *
 */
int sdr14_destroy (sdr14_t*);


// 8192 byte data block
// 

#endif /* __RFSPACE_SDR14_H__ */
