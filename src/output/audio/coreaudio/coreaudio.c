/*
 *  coreaudio.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "coreaudio.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>





#pragma mark -
#pragma mark private methods

int __coreaudio_feed (coreaudio_t*, uint32_t, uint8_t*);
OSStatus __audio_coreaudio_audiounit_cb (coreaudio_t*, AudioUnitRenderActionFlags*, const AudioTimeStamp*, uint32_t, uint32_t, AudioBufferList*);
uint64_t __coreaudio_milliseconds ();





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
coreaudio_init (coreaudio_t *coreaudio, uint32_t size, opool_t *pool)
{
	int error;
	OSStatus oserror;
	
	if (unlikely(coreaudio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null coreaudio_t");
	
	// initialize the parent class
	if (0 != (error = audio_init((audio_t*)coreaudio, "coreaudio", size, (cobject_destroy_func)coreaudio_destroy, pool)))
		LOG_ERROR_AND_RETURN(-101, "failed to audio_init(), %d", error);
	
	// allocate our buffer for holding samples
	if (NULL == (coreaudio->audio.data = malloc(sizeof(int16_t) * 1024 * 1024)))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc(%lu), %s", (sizeof(int16_t) * 1024 * 1024), strerror(errno));
	
	memset(coreaudio->audio.data, 0, sizeof(int16_t) * 1024 * 1024);
	
	// the read/write offsets
	coreaudio->audio.rd_offs = 0;
	coreaudio->audio.wr_offs = 0;
	
	coreaudio->desc.componentType = kAudioUnitType_Output;
	coreaudio->desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	coreaudio->desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	coreaudio->desc.componentFlags = 0;
	coreaudio->desc.componentFlagsMask = 0;
	
	coreaudio->input.inputProc = (AURenderCallback)__audio_coreaudio_audiounit_cb;
	coreaudio->input.inputProcRefCon = coreaudio;
	
	coreaudio->format.mSampleRate = 192000.0;
	coreaudio->format.mFormatID = kAudioFormatLinearPCM;
	coreaudio->format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kLinearPCMFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
	coreaudio->format.mBytesPerPacket = 2;
	coreaudio->format.mFramesPerPacket = 1;
	coreaudio->format.mBytesPerFrame = 2;
	coreaudio->format.mChannelsPerFrame = 2;
	coreaudio->format.mBitsPerChannel = 16;
	
	// find the default audio output device
	if (NULL == (coreaudio->comp = FindNextComponent(NULL, &coreaudio->desc)))
		LOG_ERROR_AND_RETURN(-102, "failed to FindNextComponent()");
	
	// connect to the default audio output device
	if (noErr != (oserror = OpenAComponent(coreaudio->comp, &coreaudio->unit)) || coreaudio->unit == NULL)
		LOG_ERROR_AND_RETURN(-103, "failed to OpenAComponent(), %d", oserror);
	
	// configure the connection for data via a function callback
	if (noErr != (oserror = AudioUnitSetProperty(coreaudio->unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &coreaudio->input, sizeof(coreaudio->input))))
		LOG_ERROR_AND_RETURN(-104, "failed to AudioUnitSetProperty(RenderCallback), %d", oserror);
	
	// configure the stream format for input
	if (noErr != (oserror = AudioUnitSetProperty(coreaudio->unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &coreaudio->format, sizeof(coreaudio->format))))
		LOG_ERROR_AND_RETURN(-105, "failed to AudioUnitSetProperty(StreamFormat), %d", oserror);
	
	// initialize the audio unit
	if (noErr != (oserror = AudioUnitInitialize(coreaudio->unit)))
		LOG_ERROR_AND_RETURN(-106, "failed to AudioUnitInitialize(), %d", oserror);
	
	// start the audio stream
	if (noErr != (oserror = AudioOutputUnitStart(coreaudio->unit)))
		LOG_ERROR_AND_RETURN(-107, "failed to AudioOutputUnitStart(), %d", oserror);
	
	// handle notifications
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2, false);
	
	coreaudio->audio.__feed_fp = (__audio_feed_fp_func)__coreaudio_feed;
	
	return 0;
}

/**
 *
 *
 */
int
coreaudio_destroy (coreaudio_t *coreaudio)
{
	int error;
	
	if (unlikely(coreaudio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null coreaudio_t");
	
	if (0 != (error = coreaudio_stop(coreaudio)))
		LOG_ERROR_AND_RETURN(-101, "failed to coreaudio_stop(), %d", error);
	
	// free the sample buffer
	if (coreaudio->audio.data != NULL) {
		free(coreaudio->audio.data);
		coreaudio->audio.data = NULL;
	}
	
	if (0 != (error = audio_destroy((audio_t*)coreaudio)))
		LOG_ERROR_AND_RETURN(-101, "failed to audio_destroy(), %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
audio_t*
coreaudio_audio (coreaudio_t *coreaudio)
{
	return (audio_t*)coreaudio;
}

/**
 *
 *
 */
int
coreaudio_reset (coreaudio_t *coreaudio)
{
	OSStatus oserror;
	
	if (unlikely(coreaudio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null coreaudio_t");
	
	if (unlikely(noErr != (oserror = AudioUnitReset(coreaudio->unit, kAudioUnitScope_Input, 0))))
		LOG_ERROR_AND_RETURN(-101, "failed to AudioUnitReset(), %d", oserror);
	
	return 0;
}

/**
 *
 *
 */
int
coreaudio_stop (coreaudio_t *coreaudio)
{
	OSStatus oserror;
	
	if (unlikely(coreaudio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null coreaudio_t");
	
	if (noErr != (oserror = AudioUnitReset(coreaudio->unit, kAudioUnitScope_Input, 0)))
		LOG_ERROR_AND_RETURN(-101, "failed to AudioUnitReset(), %d", oserror);
	
	if (noErr != (oserror = AudioUnitUninitialize(coreaudio->unit)))
		LOG_ERROR_AND_RETURN(-102, "failed to AudioUnitInitialize(), %d", oserror);
	
	if (noErr != (oserror = CloseComponent(coreaudio->unit)))
		LOG_ERROR_AND_RETURN(-103, "failed to CloseComponent(), %d", oserror);
	
	return 0;
}





#pragma mark -
#pragma mark datastream

/**
 * int16_t's
 *
 */
int
__coreaudio_feed (coreaudio_t *coreaudio, uint32_t size, uint8_t *data)
{
	uint32_t offset;
	uint32_t capacity = sizeof(int16_t) * 1024 * 1024;
	
	if (unlikely(coreaudio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null coreaudio_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-2, "null data");
	
	offset = coreaudio->audio.wr_offs % capacity;
	
	if (offset + size <= capacity)
		memcpy(coreaudio->audio.data + offset, data, size);
	else {
		memcpy(coreaudio->audio.data + offset, data, capacity - offset);
		memcpy(coreaudio->audio.data, data + (capacity - offset), (size - capacity - offset));
	}
	
	coreaudio->audio.wr_offs += size;
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline coreaudio_t*
coreaudio_retain (coreaudio_t *coreaudio)
{
	if (unlikely(coreaudio == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null coreaudio_t");
	
	return (coreaudio_t*)audio_retain((audio_t*)coreaudio);
}

/**
 *
 *
 */
inline void
coreaudio_release (coreaudio_t *coreaudio)
{
	if (unlikely(coreaudio == NULL))
		LOG_ERROR_AND_RETURN(, "null coreaudio_t");
	
	audio_release((audio_t*)coreaudio);
}





#pragma mark -
#pragma mark coreaudio

/**
 *
 *
 */
OSStatus
__audio_coreaudio_audiounit_cb (coreaudio_t *coreaudio, AudioUnitRenderActionFlags *flags, const AudioTimeStamp *time, uint32_t bus, uint32_t frames, AudioBufferList *data)
{
	uint32_t offset, available;
	uint32_t capacity = sizeof(int16_t) * 1024 * 1024;
	
	if (unlikely(coreaudio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null coreaudio_t");
	
	if (unlikely(data->mNumberBuffers < 2))
		LOG_ERROR_AND_RETURN(-2, "too few buffers (expected at least 3 but only got %u", data->mNumberBuffers);
	
	offset = coreaudio->audio.rd_offs % capacity;
	available = coreaudio->audio.wr_offs - coreaudio->audio.rd_offs;
	
	if (available) {
		available = MIN(available, data->mBuffers[0].mDataByteSize);
		
		if (capacity - offset >= available) {
			memcpy(data->mBuffers[0].mData, coreaudio->audio.data + offset, available);
			memcpy(data->mBuffers[1].mData, coreaudio->audio.data + offset, available);
		}
		else {
			memcpy(data->mBuffers[0].mData, coreaudio->audio.data + offset, capacity - offset);
			memcpy(data->mBuffers[0].mData, coreaudio->audio.data, available - capacity - offset);
			memcpy(data->mBuffers[1].mData, coreaudio->audio.data + offset, capacity - offset);
			memcpy(data->mBuffers[1].mData, coreaudio->audio.data, available - capacity - offset);
		}
		
		coreaudio->audio.rd_offs += available;
		
		//LOG3("  copied %u bytes from buffer; %u remaining", available, (coreaudio->audio.wr_offs - coreaudio->audio.rd_offs));
	}
	else {
		memset(data->mBuffers[0].mData, 0, data->mBuffers[0].mDataByteSize);
		memset(data->mBuffers[1].mData, 0, data->mBuffers[1].mDataByteSize);
	}
	
	return 0;
}





#pragma mark -
#pragma mark miscellaneous

/**
 * Current time in milliseconds.
 *
 */
inline uint64_t
__coreaudio_milliseconds ()
{
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	
	return (((uint64_t)tv.tv_sec * 1000000) + (uint64_t)tv.tv_usec) / 1000;
}
