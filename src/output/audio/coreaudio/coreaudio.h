/*
 *  coreaudio.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#ifndef __AUDIO_COREAUDIO_H__
#define __AUDIO_COREAUDIO_H__

#include "../audio.h"
#include <stdint.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreServices/CoreServices.h>

//
// coreaudio
//
struct coreaudio
{
	audio_t audio;                       // parent class
	
	AudioUnit unit;                      // core audio, audio unit
	Component comp;                      // component identifier
	ComponentDescription desc;           // component description
	AURenderCallbackStruct input;        // audio unit input
	AudioStreamBasicDescription format;  // input stream format
	
	//uint64_t // some sort of time offset
};
typedef struct coreaudio coreaudio_t;





/**
 * audio, data block size, pool
 */
int coreaudio_init (coreaudio_t*, uint32_t, opool_t*);

/**
 * audio
 */
int coreaudio_destroy (coreaudio_t*);





/**
 *
 */
audio_t* coreaudio_audio (coreaudio_t*);

/**
 *
 */
int coreaudio_reset (coreaudio_t*);

/**
 *
 */
int coreaudio_stop (coreaudio_t*);





/**
 *
 */
coreaudio_t* coreaudio_retain (coreaudio_t*);

/**
 *
 */
void coreaudio_release (coreaudio_t*);

#endif /* __AUDIO_COREAUDIO_H__ */
