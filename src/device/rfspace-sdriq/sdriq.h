/*
 *  sdriq.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  RF Space SDR-IQ - http://www.rfspace.com/RFSPACE/SDR-IQ.html
 *  Protocol spec - http://www.moetronix.com/files/sdriqinterfacespec100.pdf
 *
 */

#ifndef __RFSPACE_SDRIQ_H__
#define __RFSPACE_SDRIQ_H__

#include <stdint.h>
#include "../device.h"
#include "../../output/datastream.h"
#include "../../protocol/ascp/ascp.h"

#define DEVICE_RFSPACE_SDRIQ "RFSpace SDR-IQ"

//
// sdriq
//
struct sdriq
{
	device_t device;									// parent class
	ascp_t ascp;											// protocol
};
typedef struct sdriq sdriq_t;

/**
 *
 */
int sdriq_alloc (sdriq_t**, device_desc_t*);

/**
 *
 */
int sdriq_init (sdriq_t*, device_desc_t*);

/**
 *
 */
int sdriq_destroy (sdriq_t*);

/**
 *
 */
int sdriq_isme (device_desc_t*);

#endif /* __RFSPACE_SDRIQ_H__ */
