/*
 *  audio.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "audio.h"
#include "../dataobject.h"
#include "../datastream.h"
#include "../../misc/logger.h"
#include <string.h>
#include <sys/time.h>





#pragma mark -
#pragma mark prototypes

int __audio_feed (audio_t*, uint32_t, void*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
audio_init (audio_t *audio, char *name, uint32_t size, cobject_destroy_func destroy, opool_t *pool)
{
	int error;
	
	if (unlikely(audio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null audio_t");
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)audio, name, (cobject_destroy_func)destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init(), %d", error);
	
	if (unlikely(0 != (error = datastream_init(&audio->datastream, audio, size, (datastream_feed_func)__audio_feed))))
		LOG_ERROR_AND_RETURN(-102, "failed to datastream_init(), %d", error);
	
	if (unlikely(0 != (error = dspchain_init(&audio->dspchain, 10, pool))))
		LOG_ERROR_AND_RETURN(-103, "failed to dspchain_init(), %d", error);
	
	audio->samples = 0;
	
	return 0;
}

/**
 *
 *
 */
int
audio_destroy (audio_t *audio)
{
	int error;
	
	if (unlikely(audio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null audio_t");
	
	datastream_stop(&audio->datastream);
	
	if (0 != (error = datastream_destroy(&audio->datastream)))
		LOG_ERROR_AND_RETURN(-101, "failed to datastream_destroy(), %d", error);
	
	if (0 != (error = dspchain_destroy(&audio->dspchain)))
		LOG_ERROR_AND_RETURN(-102, "failed to dspchain_destroy(), %d", error);
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)audio))))
		LOG_ERROR_AND_RETURN(-103, "failed to cobject_destroy(), %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
audio_datastream (audio_t *audio, datastream_t **datastream)
{
	if (unlikely(audio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null audio_t");
	
	if (unlikely(datastream == NULL))
		LOG_ERROR_AND_RETURN(-2, "null datastream_t");
	
	*datastream = &audio->datastream;
	
	return 0;
}

/**
 *
 *
 */
int
audio_dspchain (audio_t *audio, dspchain_t **dspchain)
{
	if (unlikely(audio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null audio_t");
	
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-2, "null dspchain_t");
	
	*dspchain = &audio->dspchain;
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline audio_t*
audio_retain (audio_t *audio)
{
	if (unlikely(audio == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null audio_t");
	
	return (audio_t*)cobject_retain((cobject_t*)audio);
}

/**
 *
 *
 */
inline void
audio_release (audio_t *audio)
{
	if (unlikely(audio == NULL))
		LOG_ERROR_AND_RETURN(, "null audio_t");
	
	cobject_release((cobject_t*)audio);
}





#pragma mark -
#pragma mark datastream

/**
 *
 *
 */
int
__audio_feed (audio_t *audio, uint32_t size, void *data)
{
	int error = 0;
	
	if (unlikely(audio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null audio_t");
	
	if (unlikely(size == 0))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	// run the sample data through the dsp chain
	if (unlikely(0 != (error = dspchain_run(&audio->dspchain, &size, data))))
		LOG_ERROR_AND_RETURN(-101, "failed to dspchain_run(), %d", error);
	
	// some of the dsp processes may require a certain number of samples before they result in usable
	// data for the dsp processes following them, or the output mechanisms (ie, audios). for this 
	// reason a dsp might set the data size to zero until it has a sufficient number of samples.
	if (size != 0) {
		if (audio->__feed_fp != NULL) {
			if (unlikely(0 != (error = (*audio->__feed_fp)(audio,size,data))))
				LOG_ERROR_AND_GOTO(-102, done, "failed to audio->feed_fp, %d", error);
		}
		else if (audio->__feed_bp != NULL) {
			if (unlikely(0 != (error = audio->__feed_bp(audio,size,data))))
				LOG_ERROR_AND_GOTO(-103, done, "failed to audio->feed_bp, %d", error);
		}
		else
			LOG_ERROR_AND_GOTO(-104, done, "no feed callback");
	}
	
	audio->samples += 1;
	
done:
	return error;
}
