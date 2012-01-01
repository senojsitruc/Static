/*
 *  core.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.21.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __STATIC_CORE_H__
#define __STATIC_CORE_H__

#include <stdint.h>
#include "../device/device.h"
#include "../driver/driver.h"
#include "../dsp/dspchain.h"
#include "../dsp/fft/fft.h"
#include "../dsp/filter/chebyshev/chebyshev.h"
#include "../dsp/other/invert/invert.h"
#include "../dsp/other/scale/scale.h"
#include "../dsp/other/shift/shift.h"
#include "../misc/event/event.h"
#include "../misc/event/eventmngr.h"
#include "../misc/mem/opool.h"
#include "../protocol/protocol.h"

struct chunk;
struct ascp_message_data;
struct ascp_message_freq;
struct ascp_message_6620;
struct ascp_message_dack;
struct ascp_message_gain;
struct coreaudio;
struct average;
struct baseband;
struct hamming;
struct applefft;
struct ooura4;
struct ooura8;
struct cpx2pwr;
struct cpx2real;
struct smooth;
struct invert;
struct demodam;
struct dspchain;
struct dataobject;
struct graphhistory;
struct graphplanar;
struct cart2polar;
struct polar2cart;
struct dspfileout;
struct dspshift;
struct dspsmush;
struct dsptrim;
struct dspzero;
struct scaledouble;
struct double2sint16;
struct int2double;
struct long2double;
struct short2double;
struct chebyshev;
struct eventhandler;
struct eventnumber;

//
// core_pools
//
struct core_pools
{
	opool_t chunk;                       // chunk_t
	opool_t dataobject;                  // dataobject_t
	
	/* output - audio */
	opool_t coreaudio;                   // coreaudio_t
	
	/* output - graph */
	opool_t graphhistory;                // graphhistory_t
	opool_t graphplanar;                 // graphplanar_t
	
	/* protocol */
	opool_t ascp_message_data;           // ascp_message_data_t
	opool_t ascp_message_freq;           // ascp_message_freq_t
	opool_t ascp_message_6620;           // ascp_message_6620_t
	opool_t ascp_message_dack;           // ascp_message_dack_t
	opool_t ascp_message_gain;           // ascp_message_gain_t
	
	/* dsp */
	opool_t dspchain;                    // dspchain_t
	
	/* dsp - fft */
	opool_t applefft;                    // applefft_t
	opool_t ooura4;                      // ooura4_t
	opool_t ooura8;                      // ooura8_t
	
	/* dsp - filter */
	opool_t chebyshev;                   // chebyshev_t
	
	/* dsp - other */
	opool_t average;                     // average_t
	opool_t baseband;                    // baseband_t
	opool_t cpx2pwr;                     // cpx2pwr_t
	opool_t cpx2real;                    // cpx2real_t
	opool_t smooth;                      // smooth_t
	opool_t invert;                      // invert_t
	opool_t cart2polar;                  // cart2polar_t
	opool_t polar2cart;                  // polar2cart_t
	opool_t dspfileout;                  // dspfileout_t
	opool_t dspshift;                    // dspshift_t
	opool_t dspsmush;                    // dspsmush_t
	opool_t dsptrim;                     // dsptrim_t
	opool_t dspzero;                     // dspzero_t
	opool_t scaledouble;                 // scaledouble_t
	
	/* dsp - window */
	opool_t hamming;                     // hamming_t
	
	/* dsp - demod */
	opool_t demodam;                     // demodam_t
	
	/* dsp - type */
	opool_t double2sint16;               // double2sint16_t
	opool_t int2double;                  // int2double_t
	opool_t long2double;                 // long2double_t
	opool_t short2double;                // short2double_t
	
	/* misc - struct */
	opool_t slist_item;                  // slist_item_t
	opool_t eventhandler;                // eventhandler_t
	opool_t eventnumber;                 // eventnumber_t
};
typedef struct core_pools core_pools_t;

//
// core
//
struct core
{
	core_pools_t pools;								// core pools
	
	eventmngr_t eventmngr;						// event manager
	
	device_t *devices[50];						// devices
	driver_t *drivers[50];						// drivers
	protocol_t *protocols[50];				// protocols
	
	uint32_t device_count;						// 
	uint32_t driver_count;						// 
	uint32_t protocol_count;					// 
};
typedef struct core core_t;





/**
 *
 */
int core_init ();

/**
 *
 */
int core_destroy ();

/**
 *
 */
core_t* coreobj ();

/**
 *
 */
core_pools_t* core_pools ();

/**
 *
 */
eventmngr_t* core_eventmngr ();





/**
 * add a driver to the collection
 */
int core_driver_add (driver_t*);

/**
 * for each driver, ask it for all known devices
 */
int core_devices_findall ();

/**
 *
 */
int core_device_alloc (device_desc_t*, device_t**);

/**
 * add a device to the collection
 */
int core_device_add (device_t*);

/**
 *
 */
device_t* core_device_get (uint32_t);





/**
 *
 */
int core_chunk (struct chunk**);

/**
 *
 */
int core_dataobject (struct dataobject**);





/**
 *
 */
int core_ascp_message_data (struct ascp_message_data**);

/**
 *
 */
int core_ascp_message_freq (struct ascp_message_freq**);

/**
 *
 */
int core_ascp_message_dack (struct ascp_message_dack**);

/**
 *
 */
int core_ascp_message_gain (struct ascp_message_gain**);





/**
 * coreaudio
 */
int core_audio_coreaudio (struct coreaudio**, uint32_t);





/**
 * dspchain
 */
int core_dsp_chain (struct dspchain**, uint32_t);

/**
 * demodam
 */
int core_dsp_demod_am (struct demodam**);

/**
 * applefft, data size
 */
int core_dsp_fft_applefft (struct applefft**, uint32_t, fft_direction, fft_type);

/**
 * ooura4, data size
 */
int core_dsp_fft_ooura4 (struct ooura4**, uint32_t, fft_direction, fft_type);

/**
 * ooura8, data size
 */
int core_dsp_fft_ooura8 (struct ooura8**, uint32_t, fft_direction, fft_type);

/**
 * chebyshev, frequency cutoff, low/high pass, ripple percent, poles, max signal size
 */
int core_dsp_filter_chebyshev (struct chebyshev**, double, chebyshev_type, double, uint32_t, uint32_t);

/**
 * average, data size, average size
 */
int core_dsp_other_average (struct average**, uint32_t, uint32_t);

/**
 * baseband, frequency (in Hz)
 */
int core_dsp_other_baseband (struct baseband**, uint32_t);

/**
 * hamming, window size, iq offset
 */
int core_dsp_window_hamming (struct hamming**, uint32_t, double);

/**
 * cpx2pwr, maxdb, mindb
 */
int core_dsp_other_cpx2pwr (struct cpx2pwr**, double, double);

/**
 * cpx2real
 */
int core_dsp_other_cpx2real (struct cpx2real**);

/**
 * smooth, period (ie, smoothiness)
 */
int core_dsp_other_smooth (struct smooth**, uint32_t);

/**
 *
 */
int core_dsp_other_invert (struct invert**, invert_type);

/**
 *
 */
int core_dsp_other_cart2polar (struct cart2polar**);

/**
 *
 */
int core_dsp_other_polar2cart (struct polar2cart**);

/**
 * file path
 */
int core_dsp_other_fileout (struct dspfileout**, dsp_datatype, char*);

/**
 * scale double
 */
int core_dsp_other_scale_double (struct scaledouble**, dspscale_mode, double, double, double, double);

/**
 * shift, direction, size
 */
int core_dsp_other_shift (struct dspshift**, dspshift_dir, uint32_t);

/**
 * smush
 */
int core_dsp_other_smush (struct dspsmush**, uint32_t);

/**
 * trim
 */
int core_dsp_other_trim (struct dsptrim**, uint32_t, uint32_t);

/**
 *
 */
int core_dsp_other_zero (struct dspzero**, dspscale_mode, uint32_t, uint32_t);

/**
 *
 */
int core_dsp_type_double2sint16 (struct double2sint16**);

/**
 *
 */
int core_dsp_type_int2double (struct int2double**);

/**
 *
 */
int core_dsp_type_long2double (struct long2double**);

/**
 *
 */
int core_dsp_type_short2double (struct short2double**);





/**
 * eventnumber, sender, type
 */
int core_misc_eventnumber (struct eventnumber**, void*, uint64_t);

/**
 * eventhandler, context, sender, type
 */
int core_misc_eventhandler (struct eventhandler**, void*, void*, uint64_t, int (*)(struct eventhandler*,struct event*));





/**
 * graph history, name, data block size
 */
int core_graph_history (struct graphhistory**, char*, uint32_t);

/**
 * graph history, name, data block size
 */
int core_graph_planar (struct graphplanar**, char*, uint32_t);

#endif /* __STATIC_CORE_H__ */
