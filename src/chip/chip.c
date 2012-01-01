/*
 *  chip.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.28.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "chip.h"
#include "../misc/logger.h"

#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
chip_init (chip_t *chip, chip_type type, char *name, cobject_destroy_func destroy, opool_t *pool)
{
	int error;
	
	if (unlikely(chip == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chip_t");
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)chip, name, destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	chip->type = type;
	
	return 0;
}

/**
 *
 *
 */
int
chip_destroy (chip_t *chip)
{
	int error;
	
	if (unlikely(chip == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chip_t");
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)chip))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline chip_t*
chip_retain (chip_t *chip)
{
	if (unlikely(chip == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null chip_t");
	
	return (chip_t*)cobject_retain((cobject_t*)chip);
}

/**
 *
 *
 */
inline void
chip_release (chip_t *chip)
{
	if (unlikely(chip == NULL))
		LOG_ERROR_AND_RETURN(, "null chip_t");
	
	cobject_release((cobject_t*)chip);
}
