/*
 *  ad6620.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.28.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __CHIP_AD6620_H__
#define __CHIP_AD6620_H__

#include "../chip.h"
#include "../../misc/mem/cobject.h"
#include "../../misc/mem/opool.h"

//
// ad6620params
//
struct ad6620params
{
	uint8_t mcic2;										// 0x306
	uint8_t mcic5;										// 0x308
	uint8_t mrcf;											// 0x30A
	uint8_t scic2;										// 0x305
	uint8_t scic5;										// 0x307
	uint8_t sout;											// 0x309 - output rcf control register
	int32_t coef[256];								// 0x000-0x0FF
};
typedef struct ad6620params ad6620params_t;

//
// ad6620
//
struct ad6620
{
	chip_t chip;											// parent class
	ad6620params_t params;						// params
};
typedef struct ad6620 ad6620_t;

/**
 *
 */
int ad6620_init (ad6620_t*, opool_t*);

/**
 *
 */
int ad6620_destroy (ad6620_t*);





/**
 *
 */
chip_t* ad6620_chip (ad6620_t*);

/**
 *
 */
int ad6620_stdcoeffs (ad6620_t*, uint32_t);





/**
 *
 */
ad6620_t* ad6620_retain (ad6620_t*);

/**
 *
 */
void ad6620_release (ad6620_t*);

#endif /* __CHIP_AD6620_H__ */
