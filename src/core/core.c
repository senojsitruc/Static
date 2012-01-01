/*
 *  core.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.21.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "core.h"
#include "../device/rfspace-sdr14/sdr14.h"
#include "../device/rfspace-sdrip/sdrip.h"
#include "../device/rfspace-sdriq/sdriq.h"
#include "../driver/ftd2xx/ftd2xx.h"
#include "../driver/ftdi/ftdi.h"
#include "../dsp/demod/am/demodam.h"
#include "../dsp/fft/applefft/applefft.h"
#include "../dsp/fft/ooura4/ooura4.h"
#include "../dsp/fft/ooura8/ooura8.h"
#include "../dsp/filter/chebyshev/chebyshev.h"
#include "../dsp/other/average/average.h"
#include "../dsp/other/baseband/baseband.h"
#include "../dsp/other/cart2polar/cart2polar.h"
#include "../dsp/other/cpx2pwr/cpx2pwr.h"
#include "../dsp/other/cpx2real/cpx2real.h"
#include "../dsp/other/fileout/fileout.h"
#include "../dsp/other/invert/invert.h"
#include "../dsp/other/polar2cart/polar2cart.h"
#include "../dsp/other/scale/double.h"
#include "../dsp/other/scale/scale.h"
#include "../dsp/other/shift/shift.h"
#include "../dsp/other/smooth/smooth.h"
#include "../dsp/other/smush/smush.h"
#include "../dsp/other/trim/trim.h"
#include "../dsp/other/zero/zero.h"
#include "../dsp/type/double2sint16/double2sint16.h"
#include "../dsp/type/int2double/int2double.h"
#include "../dsp/type/long2double/long2double.h"
#include "../dsp/type/short2double/short2double.h"
#include "../dsp/window/hamming/hamming.h"
#include "../misc/event/eventhandler.h"
#include "../misc/event/eventnumber.h"
#include "../misc/logger.h"
#include "../misc/mem/opool.h"
#include "../misc/net/chunk.h"
#include "../misc/struct/slist.h"
#include "../output/dataobject.h"
#include "../output/audio/coreaudio/coreaudio.h"
#include "../output/graph/history/history.h"
#include "../output/graph/planar/planar.h"
#include "../protocol/ascp/ascp.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static core_t *gCore;

//
// device descriptions
//
static device_desc_t gDeviceDescs[] = 
{
	{ 0, 0, 0, 0, "", DEVICE_RFSPACE_SDRIQ, 1, NULL, NULL, (device_alloc_func)sdriq_alloc, (device_isme_func)sdriq_isme }
//{ 0, 0, 0, 0, "", DEVICE_RFSPACE_SDR14, 0, NULL, NULL, (device_alloc_func)sdr14_alloc, (device_isme_func)sdr14_isme },
//{ 0, 0, 0, 0, "", DEVICE_RFSPACE_SDRIP, 0, NULL, NULL, (device_alloc_func)sdrip_alloc, (device_isme_func)sdrip_isme }
};

//
// driver descriptions
//
static driver_desc_t gDriverDescs[] =
{
	{ DRIVER_FTDI,   1, (driver_alloc_func)ftdi_alloc   }
//{ DRIVER_FTD2XX, 0, (driver_alloc_func)ftd2xx_alloc }
};





#pragma mark -
#pragma mark private methods

static int core_drivers_loadall ();





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
core_init ()
{
	int error;
	core_t *core;
	
	if (unlikely(NULL == (core = gCore = (core_t*)malloc(sizeof(core_t)))))
		LOG_ERROR_AND_RETURN(-101, "failed to malloc(%ld), %s", sizeof(core_t), strerror(errno));
	
	memset(gCore, 0, sizeof(core_t));
	
	/* protocol */
	
	if (0 != (error = opool_init(&core_pools()->ascp_message_data, sizeof(ascp_message_data_t), 1000, 5, "ascp_message_data_t")))
		LOG_ERROR_AND_RETURN(-102, "failed to opool_init(ascp_message_data_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->ascp_message_freq, sizeof(ascp_message_freq_t), 100, 5, "ascp_message_freq_t")))
		LOG_ERROR_AND_RETURN(-103, "failed to opool_init(ascp_message_freq_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->ascp_message_6620, sizeof(ascp_message_6620_t), 50, 1, "ascp_message_6620_t")))
		LOG_ERROR_AND_RETURN(-104, "failed to opool_init(ascp_message_6620_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->ascp_message_dack, sizeof(ascp_message_dack_t), 50, 1, "ascp_message_dack_t")))
		LOG_ERROR_AND_RETURN(-105, "failed to opool_init(ascp_message_dack_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->ascp_message_gain, sizeof(ascp_message_gain_t), 50, 1, "ascp_message_gain_t")))
		LOG_ERROR_AND_RETURN(-106, "failed to opool_init(ascp_message_gain_t), %d", error);
	
	/* dsp */
	
	if (0 != (error = opool_init(&core_pools()->applefft, sizeof(applefft_t), 50, 1, "applefft_t")))
		LOG_ERROR_AND_RETURN(-201, "failed to opool_init(applefft_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->ooura4, sizeof(ooura4_t), 50, 1, "ooura4_t")))
		LOG_ERROR_AND_RETURN(-202, "failed to opool_init(ooura4_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->ooura8, sizeof(ooura8_t), 50, 1, "ooura8_t")))
		LOG_ERROR_AND_RETURN(-203, "failed to opool_init(ooura8_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->average, sizeof(average_t), 50, 1, "average_t")))
		LOG_ERROR_AND_RETURN(-204, "failed to opool_init(average_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->baseband, sizeof(baseband_t), 50, 1, "baseband_t")))
		LOG_ERROR_AND_RETURN(-205, "failed to opool_init(baseband_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->hamming, sizeof(hamming_t), 50, 1, "hamming_t")))
		LOG_ERROR_AND_RETURN(-206, "failed to opool_init(hamming_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->cpx2pwr, sizeof(cpx2pwr_t), 50, 1, "cpx2pwr_t")))
		LOG_ERROR_AND_RETURN(-207, "failed to opool_init(cpx2pwr_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->cpx2real, sizeof(cpx2real_t), 50, 1, "cpx2real_t")))
		LOG_ERROR_AND_RETURN(-208, "failed to opool_init(cpx2real_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->smooth, sizeof(smooth_t), 50, 1, "smooth_t")))
		LOG_ERROR_AND_RETURN(-209, "failed to opool_init(smooth_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->invert, sizeof(invert_t), 50, 1, "invert_t")))
		LOG_ERROR_AND_RETURN(-210, "failed to opool_init(invert_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->demodam, sizeof(demodam_t), 50, 1, "demodam_t")))
		LOG_ERROR_AND_RETURN(-211, "failed to opool_init(demodam_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->dspchain, sizeof(dspchain_t), 50, 1, "dspchain_t")))
		LOG_ERROR_AND_RETURN(-212, "failed to opool_init(dspchain_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->cart2polar, sizeof(cart2polar_t), 50, 1, "cart2polar_t")))
		LOG_ERROR_AND_RETURN(-213, "failed to opool_init(cart2polar_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->polar2cart, sizeof(polar2cart_t), 50, 1, "polar2cart_t")))
		LOG_ERROR_AND_RETURN(-214, "failed to opool_init(polar2cart_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->dspfileout, sizeof(dspfileout_t), 50, 1, "dspfileout_t")))
		LOG_ERROR_AND_RETURN(-215, "failed to opool_init(dspfileout_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->scaledouble, sizeof(scaledouble_t), 50, 1, "scaledouble_t")))
		LOG_ERROR_AND_RETURN(-216, "failed to opool_init(scaledouble_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->dspshift, sizeof(dspshift_t), 50, 1, "dspshift_t")))
		LOG_ERROR_AND_RETURN(-217, "failed to opool_init(dspshift_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->dspsmush, sizeof(dspsmush_t), 50, 1, "dspsmush_t")))
		LOG_ERROR_AND_RETURN(-218, "failed to opool_init(dspsmush_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->dsptrim, sizeof(dsptrim_t), 50, 1, "dsptrim_t")))
		LOG_ERROR_AND_RETURN(-219, "failed to opool_init(dsptrim_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->dspzero, sizeof(dspzero_t), 50, 1, "dspzero_t")))
		LOG_ERROR_AND_RETURN(-220, "failed to opool_init(dspzero_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->double2sint16, sizeof(double2sint16_t), 50, 1, "double2sint16_t")))
		LOG_ERROR_AND_RETURN(-221, "failed to opool_init(double2sint16_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->int2double, sizeof(int2double_t), 50, 1, "int2double_t")))
		LOG_ERROR_AND_RETURN(-222, "failed to opool_init(int2double_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->long2double, sizeof(long2double_t), 50, 1, "long2double_t")))
		LOG_ERROR_AND_RETURN(-223, "failed to opool_init(long2double_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->short2double, sizeof(short2double_t), 50, 1, "short2double_t")))
		LOG_ERROR_AND_RETURN(-224, "failed to opool_init(short2double_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->chebyshev, sizeof(chebyshev_t), 50, 1, "chebyshev_t")))
		LOG_ERROR_AND_RETURN(-225, "failed to opool_init(chebyshev_t), %d", error);
	
	/* struct */
	
	if (0 != (error = opool_init(&core_pools()->slist_item, sizeof(slist_item_t), 1000, 10, "slist_item_t")))
		LOG_ERROR_AND_RETURN(-301, "failed to opool_init(slist_item_t), %d", error);
	
	/* net */
	
	if (0 != (error = opool_init(&core_pools()->chunk, sizeof(chunk_t), 1000, 10, "chunk_t")))
		LOG_ERROR_AND_RETURN(-401, "failed to opool_init(chunk_t), %d", error);
	
	/* output */
	
	if (0 != (error = opool_init(&core_pools()->dataobject, sizeof(dataobject_t), 1000, 1, "dataobject_t")))
		LOG_ERROR_AND_RETURN(-501, "failed to opool_init(dataobject_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->coreaudio, sizeof(coreaudio_t), 50, 1, "coreaudio_t")))
		LOG_ERROR_AND_RETURN(-502, "failed to opool_init(coreaudio_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->graphhistory, sizeof(graphhistory_t), 50, 1, "graphhistory_t")))
		LOG_ERROR_AND_RETURN(-503, "failed to opool_init(graphhistory_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->graphplanar, sizeof(graphplanar_t), 50, 1, "graphplanar_t")))
		LOG_ERROR_AND_RETURN(-504, "failed to opool_init(graphplanar_t), %d", error);
	
	/* misc */
	
	if (0 != (error = opool_init(&core_pools()->eventhandler, sizeof(eventhandler_t), 50, 1, "eventhandler_t")))
		LOG_ERROR_AND_RETURN(-601, "failed to opool_init(eventhandler_t), %d", error);
	
	if (0 != (error = opool_init(&core_pools()->eventnumber, sizeof(eventnumber_t), 50, 1, "eventnumber_t")))
		LOG_ERROR_AND_RETURN(-602, "failed to opool_init(eventnumber_t), %d", error);
	
	/* other stuff */
	
	if (0 != (error = eventmngr_init(&core->eventmngr, 100, NULL)))
		LOG_ERROR_AND_RETURN(-701, "failed to eventmngr_init, %d", error);
	
	/* drivers and devices */
	
	core_drivers_loadall();
	core_devices_findall();
	
	return 0;
}

/**
 *
 *
 */
int
core_destroy ()
{
	
	
	// TODO: free the opool_t's
	
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 */
inline core_t*
coreobj ()
{
	return gCore;
}

/**
 *
 *
 */
inline core_pools_t*
core_pools ()
{
	if (gCore == NULL)
		return NULL;
	else
		return &gCore->pools;
}

/**
 *
 *
 */
inline eventmngr_t*
core_eventmngr ()
{
	if (gCore == NULL)
		return NULL;
	else
		return &gCore->eventmngr;
}





#pragma mark -
#pragma mark pool objects

/**
 *
 *
 */
int
core_chunk (chunk_t **chunk)
{
	int error;
	opool_t *pool = &core_pools()->chunk;
	
	if (unlikely(chunk == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chunk_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)chunk))))
		LOG_ERROR_AND_RETURN(-101, "failed to core_pop(chunk_t), %d", error);
	
	if (unlikely(0 != (error = chunk_init(*chunk, pool)))) {
		opool_push(pool, *chunk);
		LOG_ERROR_AND_RETURN(-102, "failed to chunk_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dataobject (struct dataobject **dataobject)
{
	int error;
	opool_t *pool = &core_pools()->dataobject;
	
	if (unlikely(dataobject == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dataobject_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)dataobject)) || *dataobject == NULL))
		LOG_ERROR_AND_RETURN(-101, "failed to core_pop(dataobject_t), %d", error);
	
	if (unlikely(0 != (error = dataobject_init(*dataobject, pool)))) {
		opool_push(pool, *dataobject);
		LOG_ERROR_AND_RETURN(-102, "failed to dataobject_init(), %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_ascp_message_data (struct ascp_message_data **data)
{
	int error;
	opool_t *pool = &core_pools()->ascp_message_data;
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_data_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)data)) || *data == NULL))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(ascp_message_data_t), %d", error);
	
	if (unlikely(0 != (error = ascp_message_init(&(*data)->header, "ascp_message_data", pool)))) {
		opool_push(pool, *data);
		LOG_ERROR_AND_RETURN(-102, "failed to ascp_message_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_ascp_message_freq (ascp_message_freq_t **freq)
{
	int error;
	opool_t *pool = &core_pools()->ascp_message_freq;
	
	if (unlikely(freq == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_freq_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)freq)) || *freq == NULL))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(ascp_message_freq_t), %d", error);
	
	if (unlikely(0 != (error = ascp_message_init(&(*freq)->header, "ascp_message_freq", pool)))) {
		opool_push(pool, *freq);
		LOG_ERROR_AND_RETURN(-102, "failed to ascp_message_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_ascp_message_gain (ascp_message_gain_t **gain)
{
	int error;
	opool_t *pool = &core_pools()->ascp_message_gain;
	
	if (unlikely(gain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_gain_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)gain)) || *gain == NULL))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(ascp_message_gain_t), %d", error);
	
	if (unlikely(0 != (error = ascp_message_init(&(*gain)->header, "ascp_message_gain", pool)))) {
		opool_push(pool, *gain);
		LOG_ERROR_AND_RETURN(-102, "failed to ascp_message_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_ascp_message_dack (ascp_message_dack_t **dack)
{
	int error;
	opool_t *pool = &core_pools()->ascp_message_dack;
	
	if (unlikely(dack == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_dack_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)dack)) || *dack == NULL))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(ascp_message_dack_t), %d", error);
	
	if (unlikely(0 != (error = ascp_message_init(&(*dack)->header, "ascp_message_dack", pool)))) {
		opool_push(pool, *dack);
		LOG_ERROR_AND_RETURN(-102, "failed to ascp_message_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_audio_coreaudio (struct coreaudio **coreaudio, uint32_t size)
{
	int error;
	opool_t *pool = &core_pools()->coreaudio;
	
	if (unlikely(coreaudio == NULL))
		LOG_ERROR_AND_RETURN(-1, "null coreaudio_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)coreaudio))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(coreaudio_t), %d", error);
	
	if (unlikely(0 != (error = coreaudio_init(*coreaudio, size, pool)))) {
		opool_push(pool, *coreaudio);
		LOG_ERROR_AND_RETURN(-102, "failed to coreaudio_init(), %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_chain (struct dspchain **dspchain, uint32_t size)
{
	int error;
	opool_t *pool = &core_pools()->dspchain;
	
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspchain_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)dspchain))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(dspchain_t), %d", error);
	
	if (unlikely(0 != (error = dspchain_init(*dspchain, size, pool)))) {
		opool_push(pool, *dspchain);
		LOG_ERROR_AND_RETURN(-102, "failed to dspchain_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_demod_am (struct demodam **demodam)
{
	int error;
	opool_t *pool = &core_pools()->demodam;
	
	if (unlikely(demodam == NULL))
		LOG_ERROR_AND_RETURN(-1, "null demodam_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)demodam))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(demodam_t), %d", error);
	
	if (unlikely(0 != (error = demodam_init(*demodam, pool)))) {
		opool_push(pool, *demodam);
		LOG_ERROR_AND_RETURN(-102, "failed to demodam_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_fft_applefft (struct applefft **applefft, uint32_t size, fft_direction direction, fft_type type)
{
	int error;
	opool_t *pool = &core_pools()->applefft;
	
	if (unlikely(applefft == NULL))
		LOG_ERROR_AND_RETURN(-1, "null applefft_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)applefft))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(applefft_t), %d", error);
	
	if (unlikely(0 != (error = applefft_init(*applefft, size, direction, type, pool)))) {
		opool_push(pool, *applefft);
		LOG_ERROR_AND_RETURN(-102, "failed to applefft_init(), %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_fft_ooura4 (struct ooura4 **ooura4, uint32_t size, fft_direction direction, fft_type type)
{
	int error;
	opool_t *pool = &core_pools()->ooura4;
	
	if (unlikely(ooura4 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura4_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)ooura4))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(ooura4_t), %d", error);
	
	if (unlikely(0 != (error = ooura4_init(*ooura4, size, direction, type, pool)))) {
		opool_push(pool, *ooura4);
		LOG_ERROR_AND_RETURN(-102, "failed to ooura4_init(), %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_fft_ooura8 (struct ooura8 **ooura8, uint32_t size, fft_direction direction, fft_type type)
{
	int error;
	opool_t *pool = &core_pools()->ooura8;
	
	if (unlikely(ooura8 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura8_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)ooura8))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(ooura8_t), %d", error);
	
	if (unlikely(0 != (error = ooura8_init(*ooura8, size, direction, type, pool)))) {
		opool_push(pool, *ooura8);
		LOG_ERROR_AND_RETURN(-102, "failed to ooura8_init, %d", error);
	}
	
	return 0;
}

/**
 * chebyshev, frequency cutoff, low/high pass, ripple percent, poles, max signal size
 */
int
core_dsp_filter_chebyshev (struct chebyshev **chebyshev, double cutoff, chebyshev_type type, double ripple, uint32_t poles, uint32_t size)
{
	int error;
	opool_t *pool = &core_pools()->chebyshev;
	
	if (unlikely(chebyshev == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chebyshev_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)chebyshev))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(chebyshev_t), %d", error);
	
	if (unlikely(0 != (error = chebyshev_init(*chebyshev, cutoff, type, ripple, poles, size, pool)))) {
		opool_push(pool, *chebyshev);
		LOG_ERROR_AND_RETURN(-102, "failed to chebyshev_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_average (struct average **average, uint32_t data_size, uint32_t avg_size)
{
	int error;
	opool_t *pool = &core_pools()->average;
	
	if (unlikely(average == NULL))
		LOG_ERROR_AND_RETURN(-1, "null average_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)average))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(average_t), %d", error);
	
	if (unlikely(0 != (error = average_init(*average, data_size, avg_size, pool)))) {
		opool_push(pool, *average);
		LOG_ERROR_AND_RETURN(-102, "failed to average_init(data_size=%u, avg_size=%u), %d", data_size, avg_size, error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_baseband (struct baseband **baseband, uint32_t frequency)
{
	int error;
	opool_t *pool = &core_pools()->baseband;
	
	if (unlikely(baseband == NULL))
		LOG_ERROR_AND_RETURN(-1, "null baseband_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)baseband))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(baseband_t), %d", error);
	
	if (unlikely(0 != (error = baseband_init(*baseband, frequency, pool)))) {
		opool_push(pool, *baseband);
		LOG_ERROR_AND_RETURN(-102, "failed to baseband_init(%u), %d", frequency, error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_window_hamming (struct hamming **hamming, uint32_t window_size, double iqoffset)
{
	int error;
	opool_t *pool = &core_pools()->hamming;
	
	if (unlikely(hamming == NULL))
		LOG_ERROR_AND_RETURN(-1, "null hamming_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)hamming))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(hamming_t), %d", error);
	
	if (unlikely(0 != (error = hamming_init(*hamming, window_size, iqoffset, pool)))) {
		opool_push(pool, *hamming);
		LOG_ERROR_AND_RETURN(-102, "failed to hamming_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_cpx2pwr (struct cpx2pwr **cpx2pwr, double maxdb, double mindb)
{
	int error;
	opool_t *pool = &core_pools()->cpx2pwr;
	
	if (unlikely(cpx2pwr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2pwr_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)cpx2pwr))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(cpx2pwr_t), %d", error);
	
	if (unlikely(0 != (error = cpx2pwr_init(*cpx2pwr, maxdb, mindb, pool)))) {
		opool_push(pool, *cpx2pwr);
		LOG_ERROR_AND_RETURN(-102, "failed to cpx2pwr_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_cpx2real (struct cpx2real **cpx2real)
{
	int error;
	opool_t *pool = &core_pools()->cpx2real;
	
	if (unlikely(cpx2real == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2real_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)cpx2real))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(cpx2real_t), %d", error);
	
	if (unlikely(0 != (error = cpx2real_init(*cpx2real, pool)))) {
		opool_push(pool, *cpx2real);
		LOG_ERROR_AND_RETURN(-102, "failed to cpx2real_init(), %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_smooth (struct smooth **smooth, uint32_t period)
{
	int error;
	opool_t *pool = &core_pools()->smooth;
	
	if (unlikely(smooth == NULL))
		LOG_ERROR_AND_RETURN(-1, "null smooth_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)smooth))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(smooth_t), %d", error);
	
	if (unlikely(0 != (error = smooth_init(*smooth, period, pool)))) {
		opool_push(pool, *smooth);
		LOG_ERROR_AND_RETURN(-102, "failed to smooth_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_invert (struct invert **invert, invert_type type)
{
	int error;
	opool_t *pool = &core_pools()->invert;
	
	if (unlikely(invert == NULL))
		LOG_ERROR_AND_RETURN(-1, "null invert_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)invert))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(invert_t), %d", error);
	
	if (unlikely(0 != (error = invert_init(*invert, type, pool)))) {
		opool_push(pool, *invert);
		LOG_ERROR_AND_RETURN(-102, "failed to invert_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_cart2polar (struct cart2polar **cart2polar)
{
	int error;
	opool_t *pool = &core_pools()->cart2polar;
	
	if (unlikely(cart2polar == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cart2polar_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)cart2polar))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(cart2polar_t), %d", error);
	
	if (unlikely(0 != (error = cart2polar_init(*cart2polar, pool)))) {
		opool_push(pool, *cart2polar);
		LOG_ERROR_AND_RETURN(-102, "failed to cart2polar_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_polar2cart (struct polar2cart **polar2cart)
{
	int error;
	opool_t *pool = &core_pools()->polar2cart;
	
	if (unlikely(polar2cart == NULL))
		LOG_ERROR_AND_RETURN(-1, "null polar2cart_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)polar2cart))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(polar2cart_t), %d", error);
	
	if (unlikely(0 != (error = polar2cart_init(*polar2cart, pool)))) {
		opool_push(pool, *polar2cart);
		LOG_ERROR_AND_RETURN(-102, "failed to polar2cart_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_fileout (struct dspfileout **dspfileout, dsp_datatype type, char *path)
{
	int error;
	opool_t *pool = &core_pools()->dspfileout;
	
	if (unlikely(dspfileout == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspfileout_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)dspfileout))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(dspfileout_t), %d", error);
	
	if (unlikely(0 != (error = dspfileout_init(*dspfileout, type, path, pool)))) {
		opool_push(pool, *dspfileout);
		LOG_ERROR_AND_RETURN(-102, "failed to dspfileout_init(%s), %d", path, error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_scale_double (struct scaledouble **scaledouble, dspscale_mode mode, double src_hi, double src_lo, double dst_hi, double dst_lo)
{
	int error;
	opool_t *pool = &core_pools()->scaledouble;
	
	if (unlikely(scaledouble == NULL))
		LOG_ERROR_AND_RETURN(-1, "null scaledouble_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)scaledouble))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(scaledouble_t), %d", error);
	
	if (unlikely(0 != (error = scaledouble_init(*scaledouble, mode, src_hi, src_lo, dst_hi, dst_lo, pool)))) {
		opool_push(pool, *scaledouble);
		LOG_ERROR_AND_RETURN(-102, "failed to scaledouble_init(), %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_shift (struct dspshift **dspshift, dspshift_dir dir, uint32_t size)
{
	int error;
	opool_t *pool = &core_pools()->dspshift;
	
	if (unlikely(dspshift == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspshift_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)dspshift))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(dspshift_t), %d", error);
	
	if (unlikely(0 != (error = dspshift_init(*dspshift, dir, size, pool)))) {
		opool_push(pool, *dspshift);
		LOG_ERROR_AND_RETURN(-102, "failed to dspshift_init(), %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_smush (struct dspsmush **dspsmush, uint32_t width)
{
	int error;
	opool_t *pool = &core_pools()->dspsmush;
	
	if (unlikely(dspsmush == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspsmush_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)dspsmush))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(dspsmush_t), %d", error);
	
	if (unlikely(0 != (error = dspsmush_init(*dspsmush, width, pool)))) {
		opool_push(pool, *dspsmush);
		LOG_ERROR_AND_RETURN(-102, "failed to dspsmush_init(%u), %d", width, error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_trim (struct dsptrim **dsptrim, uint32_t width, uint32_t offset)
{
	int error;
	opool_t *pool = &core_pools()->dsptrim;
	
	if (unlikely(dsptrim == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dsptrim_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)dsptrim))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(dsptrim_t), %d", error);
	
	if (unlikely(0 != (error = dsptrim_init(*dsptrim, width, offset, pool)))) {
		opool_push(pool, *dsptrim);
		LOG_ERROR_AND_RETURN(-102, "failed to dsptrim_init(%u, %u), %d", width, offset, error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_other_zero (struct dspzero **dspzero, dspscale_mode mode, uint32_t offset, uint32_t width)
{
	int error;
	opool_t *pool = &core_pools()->dspzero;
	
	if (unlikely(dspzero == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspzero_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)dspzero))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(dspzero_t), %d", error);
	
	if (unlikely(0 != (error = dspzero_init(*dspzero, mode, offset, width, pool)))) {
		opool_push(pool, *dspzero);
		LOG_ERROR_AND_RETURN(-102, "failed to dspzero_init(%u, %u), %d", width, offset, error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_type_double2sint16 (struct double2sint16 **double2sint16)
{
	int error;
	opool_t *pool = &core_pools()->double2sint16;
	
	if (unlikely(double2sint16 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null double2sint16_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)double2sint16))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(double2sint16_t), %d", error);
	
	if (unlikely(0 != (error = double2sint16_init(*double2sint16, pool)))) {
		opool_push(pool, *double2sint16);
		LOG_ERROR_AND_RETURN(-102, "failed to double2sint16_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_type_int2double (struct int2double **int2double)
{
	int error;
	opool_t *pool = &core_pools()->int2double;
	
	if (unlikely(int2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null int2double_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)int2double))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(int2double_t), %d", error);
	
	if (unlikely(0 != (error = int2double_init(*int2double, pool)))) {
		opool_push(pool, *int2double);
		LOG_ERROR_AND_RETURN(-102, "failed to int2double_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_type_long2double (struct long2double **long2double)
{
	int error;
	opool_t *pool = &core_pools()->long2double;
	
	if (unlikely(long2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null long2double_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)long2double))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(long2double_t), %d", error);
	
	if (unlikely(0 != (error = long2double_init(*long2double, pool)))) {
		opool_push(pool, *long2double);
		LOG_ERROR_AND_RETURN(-102, "failed to long2double_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_dsp_type_short2double (struct short2double **short2double)
{
	int error;
	opool_t *pool = &core_pools()->short2double;
	
	if (unlikely(short2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null short2double_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)short2double))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(short2double_t), %d", error);
	
	if (unlikely(0 != (error = short2double_init(*short2double, pool)))) {
		opool_push(pool, *short2double);
		LOG_ERROR_AND_RETURN(-102, "failed to short2double_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_misc_eventnumber (struct eventnumber **eventnumber, void *sender, uint64_t type)
{
	int error;
	opool_t *pool = &core_pools()->eventnumber;
	
	if (unlikely(eventnumber == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventnumber_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)eventnumber))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(eventnumber_t), %d", error);
	
	if (unlikely(0 != (error = eventnumber_init(*eventnumber, sender, type, pool)))) {
		opool_push(pool, *eventnumber);
		LOG_ERROR_AND_RETURN(-102, "failed to eventnumber_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_misc_eventhandler (struct eventhandler **eventhandler, void *context, void *sender, uint64_t type, int (*callback)(struct eventhandler*,struct event*))
{
	int error;
	opool_t *pool = &core_pools()->eventhandler;
	
	if (unlikely(eventhandler == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventhandler_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)eventhandler))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(eventhandler_t), %d", error);
	
	if (unlikely(0 != (error = eventhandler_init(*eventhandler, context, sender, type, callback, pool)))) {
		opool_push(pool, *eventhandler);
		LOG_ERROR_AND_RETURN(-102, "failed to eventhandler_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_graph_history (struct graphhistory **graphhistory, char *name, uint32_t size)
{
	int error;
	opool_t *pool = &core_pools()->graphhistory;
	
	if (unlikely(graphhistory == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphhistory_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)graphhistory))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(graphhistory_t), %d", error);
	
	if (unlikely(0 != (error = graphhistory_init(*graphhistory, name, size, pool)))) {
		opool_push(pool, *graphhistory);
		LOG_ERROR_AND_RETURN(-102, "failed to graphhistory_init, %d", error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_graph_planar (struct graphplanar **graphplanar, char *name, uint32_t size)
{
	int error;
	opool_t *pool = &core_pools()->graphplanar;
	
	if (unlikely(graphplanar == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	if (unlikely(0 != (error = opool_pop(pool, (void**)graphplanar))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop(graphplanar_t), %d", error);
	
	if (unlikely(0 != (error = graphplanar_init(*graphplanar, name, size, GRAPH_PLANAR_MODE_CUR, pool)))) {
		opool_push(pool, *graphplanar);
		LOG_ERROR_AND_RETURN(-102, "failed to graphplanar_init, %d", error);
	}
	
	return 0;
}





#pragma mark -
#pragma mark drivers

/**
 * Iterate through all of the defined driver descriptions and allocate an instance of each driver.
 * The allocated driver instance will be stored in the drivers list of the core_t. Only enabled
 * drivers are allocated. If anything goes wrong will allocating / initializing a driver, it is
 * skipped and we move on to the next driver. We try not to fail completely if something goes wrong.
 *
 * To load your own driver, allocate it, initialize it, and call core_driver_add().
 *
 */
static int
core_drivers_loadall ()
{
	int i, count, error;
	
	count = sizeof(gDeviceDescs) / sizeof(device_desc_t);
	
	LOG3("loading %d driver description(s)", count);
	
	for (i = 0; i < count; ++i) {
		driver_desc_t *desc = &gDriverDescs[i];
		driver_t *driver = NULL;
		
		if (unlikely(0 == desc->enabled))
			LOG_ERROR_AND_CONTINUE("[%s] driver is disabled; skipping.", desc->name);
		
		if (unlikely(NULL == desc->alloc_func))
			LOG_ERROR_AND_CONTINUE("[%s] null allocation function; skipping.", desc->name);
		
		if (unlikely(0 != (error = (*desc->alloc_func)(&driver))))
			LOG_ERROR_AND_CONTINUE("[%s] failed to allocate driver, %d", desc->name, error);
		
		if (unlikely(0 != (error = core_driver_add(driver))))
			LOG_ERROR_AND_CONTINUE("[%s] failed to core_driver_add, %d", desc->name, error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_driver_add (driver_t *driver)
{
	if (unlikely(driver == NULL))
		LOG_ERROR_AND_RETURN(-1, "null driver_t");
	
	gCore->drivers[gCore->driver_count++] = driver;
	
	LOG3("[%s] added driver [count=%u]", driver->desc.name, gCore->driver_count);
	
	return 0;
}





#pragma mark -
#pragma mark devices

/**
 * For each driver, ask it for all of its known devices. For each device description that we get
 * from each driver, allocate an instance of the corresponding device and add it to the collection.
 *
 */
int
core_devices_findall ()
{
	__block int error;
	
	for (uint32_t i = 0; i < gCore->driver_count; ++i) {
		driver_t *driver = gCore->drivers[i];
		
		if (unlikely(driver == NULL))
			continue;
		
		if (unlikely(0 != (error = driver_findall(driver, ^ int (device_desc_t *desc){
			__block device_t *device;
			
			if (unlikely(desc == NULL))
				LOG_ERROR_AND_RETURN(-101, "null device_desc_t");
			
			if (unlikely(0 != (error = core_device_alloc(desc, &device))))
				LOG_ERROR_AND_RETURN(-102, "failed to core_device_alloc(%s), %d", desc->name, error);
			
			core_device_add(device);
			
			return 0;
		}))))
			LOG_ERROR_AND_CONTINUE("[%s] failed to driver_findall, %d", driver->desc.name, error);
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_device_alloc (device_desc_t *desc, device_t **device)
{
	int i, count, error;
	
	if (unlikely(desc == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_desc_t");
	
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-2, "null device return pointer");
	
	*device = NULL;
	count = sizeof(gDeviceDescs) / sizeof(device_desc_t);
	
	for (i = 0; i < count; ++i) {
		device_desc_t *device_desc = &gDeviceDescs[i];
		
		if (device_desc->isme == NULL)
			LOG_ERROR_AND_CONTINUE("[%s] skipping because it lacks isme support", device_desc->name);
		
		if (0 > (error = (*device_desc->isme)(desc)))
			{ LOG_ERROR_AND_CONTINUE("[%s] skipping because isme failed", device_desc->name); }
		else if (error == 0)
			continue;
		
		if (unlikely(0 != (error = (*device_desc->alloc)(device, desc))))
			LOG_ERROR_AND_CONTINUE("[%s] failed to alloc, %d", device_desc->name, error);
		
		break;
	}
	
	return 0;
}

/**
 *
 *
 */
int
core_device_add (device_t *device)
{
	if (unlikely(device == NULL))
		LOG_ERROR_AND_RETURN(-1, "null device_t");
	
	gCore->devices[gCore->device_count++] = device;
	
	LOG3("[%s] added device [count=%u]", device->desc.name, gCore->device_count);
	
	return 0;
}

/**
 *
 *
 */
device_t*
core_device_get (uint32_t index)
{
	if (index >= gCore->device_count)
		LOG_ERROR_AND_RETURN(NULL, "index (%d) exceeds list bounds (%d)", index, gCore->device_count);
	
	return gCore->devices[index];
}
