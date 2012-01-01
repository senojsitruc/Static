/*
 *  scale.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.28.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __DSP_SCALE_H__
#define __DSP_SCALE_H__

//
// dspscale_mode
//
typedef enum
{
	DSPSCALE_MODE_LIN = (1 << 0),        // linear
	DSPSCALE_MODE_LOG = (1 << 1),        // log
	DSPSCALE_MODE_EXP = (1 << 2)         // exponential
} dspscale_mode;

#endif /* __DSP_SCALE_H__ */
