/*
 *  audio.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#ifndef __OUTPUT_AUDIO_H__
#define __OUTPUT_AUDIO_H__

#include "../datastream.h"
#include "../../dsp/dspchain.h"
#include "../../misc/mem/cobject.h"
#include "../../misc/mem/opool.h"

//
// audio
//
struct audio
{
	cobject_t audio;                     // parent class
	
	uint64_t samples;                    // number of samples
	void *data;                          // sample buffer
	uint32_t rd_offs;                    // read offset
	uint32_t wr_offs;                    // write offset
	
	dspchain_t dspchain;                 // dsp chain
	datastream_t datastream;             // data stream
	
	/* feed */
	int (*__feed_fp)(struct audio*, uint32_t, void*);
	int (^__feed_bp)(struct audio*, uint32_t, void*);
};
typedef struct audio audio_t;





//
// audio, size, data
//
typedef int (*__audio_feed_fp_func)(struct audio*, uint32_t, void*);
typedef int (^__audio_feed_bp_func)(struct audio*, uint32_t, void*);





/**
 *
 */
int audio_init (audio_t*, char*, uint32_t, cobject_destroy_func, opool_t*);

/**
 *
 */
int audio_destroy (audio_t*);





/**
 * Get the datastream_t instance for this audio.
 */
int audio_datastream (audio_t*, datastream_t**);

/**
 * Get the dspchain_t instance for this audio.
 */
int audio_dspchain (audio_t*, dspchain_t**);





/**
 *
 */
audio_t* audio_retain (audio_t*);

/**
 *
 */
void audio_release (audio_t*);

#endif /* __OUTPUT_AUDIO_H__ */
