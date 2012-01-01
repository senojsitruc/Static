/*
 *  sdriq.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "sdriq.h"
#include "../../chip/chip.h"
#include "../../chip/ad6620/ad6620.h"
#include "../../core/core.h"
#include "../../driver/driver.h"
#include "../../misc/atomic.h"
#include "../../misc/logger.h"
#include "../../misc/event/event.h"
#include "../../misc/event/eventmngr.h"
#include "../../misc/event/eventnumber.h"
#include "../../output/dataobject.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>





#pragma mark -
#pragma mark private methods

static int __sdriq_data_beg (sdriq_t*);
static int __sdriq_data_end (sdriq_t*);
static int __sdriq_frequency_set (sdriq_t*, uint32_t);
static int __sdriq_frequency_get (sdriq_t*);
static int __sdriq_gain_set (sdriq_t*, int32_t);
static int __sdriq_span_set (sdriq_t*, uint32_t);
static int __sdriq_message_read (sdriq_t*);
static int __sdriq_connect (sdriq_t*);
static int __sdriq_disconnect (sdriq_t*);
static int __sdriq_program (sdriq_t*, chip_t*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
sdriq_alloc (sdriq_t **sdriq, device_desc_t *desc)
{
	int error;
	
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	*sdriq = NULL;
	
	if (unlikely(NULL == (*sdriq = (sdriq_t*)malloc( sizeof(sdriq_t) ))))
		LOG_ERROR_AND_RETURN(-101, "failed to malloc, %s", strerror(errno));
	
	memset(*sdriq, 0, sizeof(sdriq_t));
	
	if (unlikely(0 != (error = sdriq_init(*sdriq, desc))))
		LOG_ERROR_AND_RETURN(-102, "failed to sdriq_init, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
sdriq_init (sdriq_t *sdriq, device_desc_t *desc)
{
	int error;
	
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device_desc_t");
	
	if (unlikely(0 != (error = ascp_init(&sdriq->ascp))))
		LOG_ERROR_AND_RETURN(-101, "failed to ascp_init, %d", error);
	
	if (unlikely(0 != (error = device_init((device_t*)sdriq, desc))))
		LOG_ERROR_AND_RETURN(-102, "failed to device_init, %d", error);
	
	sdriq->device.__message_read = (device_message_read_func)__sdriq_message_read;
	sdriq->device.__data_beg = (device_data_beg_func)__sdriq_data_beg;
	sdriq->device.__data_end = (device_data_end_func)__sdriq_data_end;
	sdriq->device.__frequency_set = (device_frequency_set_func)__sdriq_frequency_set;
	sdriq->device.__frequency_get = (device_frequency_get_func)__sdriq_frequency_get;
	sdriq->device.__gain_set = (device_gain_set_func)__sdriq_gain_set;
	sdriq->device.__span_set = (device_span_set_func)__sdriq_span_set;
	sdriq->device.__connect = (device_connect_func)__sdriq_connect;
	sdriq->device.__disconnect = (device_disconnect_func)__sdriq_disconnect;
	sdriq->device.__program = (device_program_func)__sdriq_program;
	
	return 0;
}

/**
 *
 *
 */
int
sdriq_destroy (sdriq_t *sdriq)
{
	int error;
	
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	if (unlikely(0 != (error = ascp_destroy(&sdriq->ascp))))
		LOG_ERROR_AND_RETURN(-101, "failed to ascp_destroy, %d", error);
	
	if (unlikely(0 != (error = device_destroy((device_t*)sdriq))))
		LOG_ERROR_AND_RETURN(-102, "failed to device_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark device overloads

/**
 *
 *
 */
static int
__sdriq_data_beg (sdriq_t *sdriq)
{
	int error;
	ascp_message_data_t *message;
	
	if (unlikely(0 != (error = core_ascp_message_data(&message))))
		LOG_ERROR_AND_RETURN(-101, "failed to core_ascp_message_data, %d", error);
	
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-102, "null ascp_message_data_t");
	
	sdriq->device.data_on = 1;
	
//ascp_data_get(message, ASCP_DATA_BEG, ASCP_DATA_ONE, 1);
	ascp_data_get(message, ASCP_DATA_BEG, ASCP_DATA_CON, 0);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)message, &sdriq->device.connection);
	
	return 0;
}

/**
 *
 *
 */
static int
__sdriq_data_end (sdriq_t *sdriq)
{
	int error;
	ascp_message_data_t *message;
	
	if (unlikely(0 != (error = core_ascp_message_data(&message))))
		LOG_ERROR_AND_RETURN(-101, "failed to core_ascp_message_data, %d", error);
	
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-102, "null ascp_message_data_t");
	
	sdriq->device.data_on = 0;
	
	ascp_data_get(message, ASCP_DATA_END, ASCP_DATA_CON, 0);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)message, &sdriq->device.connection);
	
	return 0;
}

/**
 *
 *
 */
static int
__sdriq_frequency_set (sdriq_t *sdriq, uint32_t frequency)
{
	ascp_message_freq_t message;
	
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	sdriq->device.frequency = frequency;
	
	ascp_frequency_set(&message, frequency);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	
	return 0;
}

/**
 *
 *
 */
static int
__sdriq_frequency_get (sdriq_t *sdriq)
{
	ascp_message_freq_t message;
	
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	ascp_frequency_get(&message);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	
	return 0;
}

/**
 *
 *
 */
static int
__sdriq_gain_set (sdriq_t *sdriq, int32_t gain)
{
	ascp_message_gain_t message;
	
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	sdriq->device.gain = (int8_t)gain;
	
	ascp_gain_set(&message, 1, (int8_t)gain);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	
	return 0;
}

/**
 *
 *
 */
static int
__sdriq_span_set (sdriq_t *sdriq, uint32_t span)
{
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	int error;
	ad6620_t ad6620;
	
	sdriq->device.span = span;
	
	ad6620_init(&ad6620, NULL);
	
	if (unlikely(0 != (error = ad6620_stdcoeffs(&ad6620, span))))
		LOG_ERROR_AND_RETURN(-101, "failed to ad6620_stdcoeffs, %d", error);
	
	if (0 != (error = device_program((device_t*)sdriq, ad6620_chip(&ad6620))))
		LOG_ERROR_AND_RETURN(-102, "failed to device_program, %d", error);
	
	ad6620_destroy(&ad6620);
	
	return 0;
}

/**
 *
 *
 */
static int
__sdriq_program (sdriq_t *sdriq, chip_t *chip)
{
	ad6620_t *ad6620 = (ad6620_t*)chip;
	ascp_message_6620_t message;
	useconds_t time = 4000;
	
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	if (unlikely(chip == NULL))
		LOG_ERROR_AND_RETURN(-2, "null chip_t");
	
	if (unlikely(CHIP_AD6620 != chip->type))
		LOG_ERROR_AND_RETURN(-3, "unsupported chip, 0x%04X", (int)chip->type);
	
	memset(&message, 0, sizeof(ascp_message_6620_t));
	
	// mode control register
	ascp_ad6620_set(&message, 0x300, 0x1);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(10000);
	
	// rcf coefficients
	for (uint16_t i = 0; i < 256; ++i) {
		ascp_ad6620_set(&message, i, ad6620->params.coef[i]);
		ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
		usleep(time);
	}
	
	// nco control register
	ascp_ad6620_set(&message, 0x301, 0x0);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// nco sync control register
	ascp_ad6620_set(&message, 0x302, -1);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// nco frequency (NCO_FREQ)
	ascp_ad6620_set(&message, 0x303, 0x0);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// nco phase offset (PHASE_OFFSET)
	ascp_ad6620_set(&message, 0x304, 0x0);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// input/cic2 scale register
	ascp_ad6620_set(&message, 0x305, ad6620->params.scic2);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// mcic2 - 1 ... decimation minus one
	ascp_ad6620_set(&message, 0x306, ad6620->params.mcic2-1);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// scic5 scale register
	ascp_ad6620_set(&message, 0x307, ad6620->params.scic5);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// mcic5 - 1 ... decimation minus one
	ascp_ad6620_set(&message, 0x308, ad6620->params.mcic5-1);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// output / rcf control register
	ascp_ad6620_set(&message, 0x309, ad6620->params.sout);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// mrcf - 1 ... decimation minus one
	ascp_ad6620_set(&message, 0x30A, ad6620->params.mrcf-1);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// rcf address offset register
	ascp_ad6620_set(&message, 0x30B, 0x0);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// taps - 1 ... number of taps minus one
	ascp_ad6620_set(&message, 0x30C, 0xFF);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// reserved
	ascp_ad6620_set(&message, 0x30D, 0x0);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(time);
	
	// update the device's decimation and samplerate values
	sdriq->device.decimation = ad6620->params.scic2 * ad6620->params.scic5 * ad6620->params.mrcf;
	sdriq->device.samplerate = 66666666. / sdriq->device.decimation;
	
	// frequency
	// http://tf.nist.gov/stations/wwv.html
	device_frequency_set((device_t*)sdriq, sdriq->device.frequency);
	usleep(time);
	
	// gain
	device_gain_set((device_t*)sdriq, sdriq->device.gain);
	usleep(time);
	
	// mode control register
	ascp_ad6620_set(&message, 0x300, 0x0);
	ascp_message_send(&sdriq->ascp, (ascp_message_t*)&message, &sdriq->device.connection);
	usleep(100000);
	
	return 0;
}

/**
 *
 *
 */
static int
__sdriq_message_read (sdriq_t *sdriq)
{
	int error = 0;
	uint32_t count, j, k;
	ascp_message_t *message = NULL;
	
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	if (unlikely(0 != (error = ascp_message_read(&sdriq->ascp, &message, &sdriq->device.connection))))
		LOG_ERROR_AND_RETURN(-101, "failed to ascp_message_read(), %d", error);
	
	// unsupported message types result in a null message
	if (message == NULL)
		return 0;
	
	switch (message->code) {
		case ASCP_CODE_DATA:
			{
				ascp_message_data_t *m = (ascp_message_data_t*)message;
				dataobject_t *dataobject = NULL;
				
				// grab a new dataobject_t instance, convert the read data into doubles and place them in
				// the data object. then, for each data stream, hand it the data object. finally, release
				// the data object so that it can be re-used.
				if (m->data[0] != 0) {
					sdriq->device.samples += 1;
					
					if (0 == sdriq->device.samples % 1000)
						LOG3("sdriq->device.samples = %llu", sdriq->device.samples);
					
					if (unlikely(0 != (error = core_dataobject(&dataobject)) || dataobject == NULL))
						LOG_ERROR_AND_GOTO(-102, done, "failed to core_dataobject(), %d", error);
					
					memcpy(dataobject->data, m->data, MIN(sizeof(dataobject->data),sizeof(m->data)));
					dataobject->size = MIN(sizeof(dataobject->data),sizeof(m->data));
					
					count = sizeof(sdriq->device.datastreams) / sizeof(void*);
					
					for (k=0, j=0; k < count && j < sdriq->device.datastreamcnt; ++k) {
						if (sdriq->device.datastreams[k] != NULL) {
							datastream_feed(sdriq->device.datastreams[k], dataobject);
							j++;
						}
					}
					
					dataobject_release(dataobject);
					dataobject = NULL;
				}
				
				break;
			}
		case ASCP_CODE_FREQ:
			{
				eventnumber_t *event = NULL;
				ascp_message_freq_t *m = (ascp_message_freq_t*)message;
				
				LOG3("[%s] got a frequency message!", sdriq->device.desc.name);
				
				sdriq->device.frequency = m->frequency;
				
				if (unlikely(0 == (error = core_misc_eventnumber(&event, sdriq, EVENT_DEVICE_FREQ)) && event != NULL)) {
					event->val.uint32val = m->frequency;
					
					if (unlikely(0 != (error = eventmngr_event_post(core_eventmngr(), (event_t*)event))))
						LOG3("[%s] failed to eventmngr_event_post(EVENT_DEVICE_FREQ), %d", sdriq->device.desc.name, error);
					
					eventnumber_release(event);
					event = NULL;
				}
				else
					LOG3("[%s] failed to core_misc_eventnumber(), %d", sdriq->device.desc.name, error);
				
				break;
			}
		case ASCP_CODE_DACK:
			{
				// programming ack
				break;
			}
		default:
			LOG3("[%s] unsupported message type, 0x%04X", sdriq->device.desc.name, message->code);
			break;
	};
	
done:
	if (message != NULL)
		ascp_message_release(message);
	
	return error;
}





#pragma mark -
#pragma mark other

/**
 *
 *
 */
int
sdriq_isme (device_desc_t *desc)
{
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_desc_t");
	
	if (0 == strcmp(desc->name, "SDR-IQ"))
		return 1;
	
	return 0;
}





#pragma mark -
#pragma mark callbacks

/**
 *
 *
 */
static int
__sdriq_connect (sdriq_t *sdriq)
{
	if (unlikely(sdriq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sdriq_t");
	
	LOG3("[%s]", sdriq->device.desc.name);
	
	return 0;
}

/**
 *
 *
 */
static int
__sdriq_disconnect (sdriq_t *sdriq)
{
	LOG3("[%s]", sdriq->device.desc.name);
	
	return 0;
}
