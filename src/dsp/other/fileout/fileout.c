/*
 *  fileout.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.19.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "fileout.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DSP_FILEOUT_LINE 100000





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
dspfileout_init (dspfileout_t *dspfileout, dsp_datatype type, char *path, opool_t *pool)
{
	int error;
	size_t path_l;
	
	if (unlikely(dspfileout == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspfileout_t");
	
	if (unlikely(path == NULL || 0 == (path_l = strlen(path))))
		LOG_ERROR_AND_RETURN(-2, "null or zero-length path");
	
	if (unlikely(path_l >= sizeof(dspfileout->path)))
		LOG_ERROR_AND_RETURN(-3, "path length exceeds max (%lu)", sizeof(dspfileout->path));
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)dspfileout, "dsp-other-dspfileout", (cobject_destroy_func)dspfileout_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	if (unlikely(NULL == (dspfileout->line = malloc(DSP_FILEOUT_LINE))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc(), %s", strerror(errno));
	
	if (unlikely(0 > (dspfileout->fd = open(path, (O_WRONLY|O_APPEND|O_CREAT), (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)))))
		LOG_ERROR_AND_RETURN(-103, "failed to open(%s), %s", path, strerror(errno));
	
	memset(dspfileout->line, 0, DSP_FILEOUT_LINE);
	memcpy(dspfileout->path, path, path_l+1);
	
	dspfileout->type = type;
	dspfileout->width = 32;
	
	dspfileout->dsp.__feed = (__dsp_feed_func)dspfileout_feed;
	dspfileout->dsp.__reset = (__dsp_reset_func)dspfileout_reset;
	
	return 0;
}

/**
 *
 *
 */
int
dspfileout_destroy (dspfileout_t *dspfileout)
{
	int error;
	
	if (unlikely(dspfileout == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspfileout_t");
	
	if (dspfileout->line != NULL) {
		free(dspfileout->line);
		dspfileout->line = NULL;
	}
	
	if (dspfileout->fd != 0) {
		close(dspfileout->fd);
		dspfileout->fd = 0;
	}
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)dspfileout))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
dspfileout_feed (dspfileout_t *dspfileout, uint32_t *size, void *data)
{
	char *line;
	int leng, temp;
	uint32_t width;
	
	if (unlikely(dspfileout == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspfileout_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	leng = DSP_FILEOUT_LINE;
	line = dspfileout->line;
	width = dspfileout->width;
	
	// double
	if (DSP_DATA_TYPE_DOUBLE == dspfileout->type) {
		double val, min, max, *dataptr = (double *)data;
		uint32_t i, count = *size / sizeof(double);
		
		min = *dataptr;
		max = *dataptr;
		
		for (i = 0; i < count && leng > 0; ++i, ++dataptr) {
			if (i != 0 && 0 == (i % width)) {
				snprintf(line, (size_t)leng, "\n");
				line += 1;
				leng -= 1;
			}
			
			val = *dataptr;
			
			if (*dataptr >= 0.)
				temp = snprintf(line, (size_t)leng, "%4u:%8.5f\t", i, val);
			else
				temp = snprintf(line, (size_t)leng, "%4u:%9.5f\t", i, val);
			
			if (val > max)
				max = val;
			
			if (val < min)
				min = val;
			
			if (unlikely(0 >= temp))
				break;
			else {
				line += temp;
				leng -= temp;
			}
		}
		
		if (0 != (i % width) || 0 == (count % width)) {
			snprintf(line, (size_t)leng, "\n");
			line += 1;
			leng -= 1;
		}
		
		temp = snprintf(line, (size_t)leng, "min=%f, max=%f", min, max);
		line += temp;
		leng -= temp;
	}
	
	// sint16
	else if (DSP_DATA_TYPE_SINT16 == dspfileout->type) {
		int16_t *dataptr = (int16_t*)data;
		uint32_t i, count = *size / sizeof(int16_t);
		
		for (i = 0; i < count && leng > 0; ++i, ++dataptr) {
			if (i != 0 && 0 == (i % width)) {
				snprintf(line, (size_t)leng, "\n");
				line += 1;
				leng -= 1;
			}
			
			if (*dataptr >= 0)
				temp = snprintf(line, (size_t)leng, "%4u:+%5hd\t", i, *dataptr);
			else
				temp = snprintf(line, (size_t)leng, "%4u:%5hd\t", i, *dataptr);
			
			if (unlikely(0 >= temp))
				break;
			else {
				line += temp;
				leng -= temp;
			}
		}
	}
	
	temp = snprintf(line, (size_t)leng, "\n");
	leng -= temp;
	
	write(dspfileout->fd, dspfileout->line, (size_t)(DSP_FILEOUT_LINE-leng));
	
	return 0;
}

/**
 *
 *
 */
int
dspfileout_reset (dspfileout_t *dspfileout)
{
	if (unlikely(dspfileout == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspfileout_t");
	
	
	// TODO: clear out the file?
	
	
	return 0;
}	



#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline dspfileout_t*
dspfileout_retain (dspfileout_t *dspfileout)
{
	if (unlikely(dspfileout == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null dspfileout_t");
	
	return (dspfileout_t*)dsp_retain((dsp_t*)dspfileout);
}

/**
 *
 *
 */
inline void
dspfileout_release (dspfileout_t *dspfileout)
{
	if (unlikely(dspfileout == NULL))
		LOG_ERROR_AND_RETURN(, "null dspfileout_t");
	
	dsp_release((dsp_t*)dspfileout);
}

