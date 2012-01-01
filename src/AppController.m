//
//  AppController.m
//  Static
//
//  Created by Curtis Jones on 2009.12.20.
//  Copyright 2009 Curtis Jones. All rights reserved.
//

#import "AppController.h"
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extern/ftd2xx/ftd2xx.h"
#include "extern/ftd2xx/WinTypes.h"
#include "extern/ftdi/ftdi.h"
#include "chip/chip.h"
#include "chip/ad6620/ad6620.h"
#include "core/core.h"
#include "dsp/dspchain.h"
#include "dsp/demod/am/demodam.h"
#include "dsp/fft/fft.h"
#include "dsp/fft/applefft/applefft.h"
#include "dsp/filter/chebyshev/chebyshev.h"
#include "dsp/other/average/average.h"
#include "dsp/other/baseband/baseband.h"
#include "dsp/other/cpx2pwr/cpx2pwr.h"
#include "dsp/other/cpx2real/cpx2real.h"
#include "dsp/other/fileout/fileout.h"
#include "dsp/other/smush/smush.h"
#include "dsp/other/trim/trim.h"
#include "dsp/other/smooth/smooth.h"
#include "dsp/other/invert/invert.h"
#include "dsp/type/short2double/short2double.h"
#include "dsp/window/hamming/hamming.h"
#include "misc/dump.h"
#include "misc/logger.h"
#include "misc/event/event.h"
#include "misc/event/eventhandler.h"
#include "misc/event/eventmngr.h"
#include "misc/event/eventnumber.h"
#include "misc/mem/memlock.h"
#include "platform/cocoa/PlanarGraphView.h"
#include "platform/cocoa/HistoryGraphView.h"
#include "protocol/ascp/ascp.h"

AppController *gAppController;
device_t *gDevice;
double iqoffset;

/**
 *
 *
 */
void
sighandler (int sig)
{
	if (gAppController != nil)
		[gAppController stop];
	
	if (gDevice != NULL) {
		device_disconnect(gDevice);
		gDevice = NULL;
	}
}

/**
 *
 *
 */
static double
calc_offset_iq ()
{
	int taps = 256;
	double iqoffsetconst = 307.0;
	int ifgain = 24;
	double div;
	
	switch(ifgain & 0x1F)
	{
		case 0:
			div = 16.0;
			break;
		case 6:
			div = 8.0;
			break;
		case 12:
			div = 4.0;
			break;
		case 18:
			div = 2.0;
			break;
		case 24:
			div = 1.0;
			break;
	}
	
	return ((double)(taps)/(div*iqoffsetconst));
}

/**
 *
 *
 */
static int
event_callback (eventhandler_t *eventhandler, event_t *event)
{
	AppController *appController = (AppController*)eventhandler->context;
	eventnumber_t *eventnumber = (eventnumber_t*)event;
	
	if (appController == nil)
		return 0;
	
	if (eventnumber->event.type == EVENT_DEVICE_FREQ) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		LOG3("frequency = %u", eventnumber->val.uint32val);
		
//	appController.planar->graph.axis_x.unit_high = eventnumber->val.uint32val + (190000. / 2.);
//	appController.planar->graph.axis_x.unit_low = eventnumber->val.uint32val - (190000. / 2.);
		appController.planar->graph.axis_x.unit_high = eventnumber->val.uint32val + (95000. / 2.);
		appController.planar->graph.axis_x.unit_low = eventnumber->val.uint32val - (95000. / 2.);
		
		graphplanar_reset(appController.planar);
//	graphhistory_shift( <frequency shift> );
		
		device_t *device = appController.device;
		
		if (device != NULL)
			[appController.graphController setLabel:[NSString stringWithFormat:@"%d KHz span, %d dB gain, %lu Hz", device->span, (int)device->gain, device->frequency]];
		
		[appController.deviceController setFrequency:eventnumber->val.uint32val];
		
		[pool release];
	}
	
	return 0;
}

@implementation AppController

@synthesize deviceController = mDeviceController;
@synthesize graphController = mGraphController;
@synthesize device = mDevice;
@synthesize planar = mPlanar;
@synthesize planar2 = mPlanar2;
@synthesize history = mHistory;





#pragma mark -
#pragma mark Structors

/**
 *
 *
 */
- (void)dealloc
{
	[super dealloc];
}

/**
 *
 *
 */
- (void)stop
{
	mStop = TRUE;
}

/**
 *
 *
 */
- (void)awakeFromNib
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	
	iqoffset = calc_offset_iq();
	gAppController = self;
	mStop = FALSE;
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(doHandleAD6620Params:) name:@"AD6620Params" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(doHandleAD6620Start:) name:@"AD6620Start" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(doHandleAD6620Stop:) name:@"AD6620Stop" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(doHandleTerminate:) name:@"NSApplicationWillTerminateNotification" object:nil];
	
	memlock_setup();
	core_init();
//[self setup];
	
	[NSThread detachNewThreadSelector:@selector(setup) toTarget:self withObject:nil];
}





#pragma mark -
#pragma mark Actions

/**
 * Info window for RFSpace SDR-IQ
 *
 */
- (IBAction)doActionInfo:(id)sender
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	[NSApp beginSheet:mDeviceController.window modalForWindow:mGraphController.window modalDelegate:self didEndSelector:@selector(doActionInfoClosed:returnCode:contextInfo:) contextInfo:nil];
}

/**
 * Called to dismiss the modal configuration sheet.
 *
 */
- (IBAction)doActionInfoClose:(id)sender
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	[NSApp endSheet:mDeviceController.window];
}

/**
 * Called after dismissing the modal configuration sheet.
 *
 */
- (void)doActionInfoClosed:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	[sheet orderOut:self];
}





#pragma mark -
#pragma mark Notifications

/**
 *
 *
 */
- (void)doHandleAD6620Start:(NSNotification *)notification
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	
	int error;
	
	if (0 != (error = device_span_set(mDevice, mDevice->span))) {
		LOG3("failed to device_span_set, %d", error);
		[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Stopped" object:nil];
		return;
	}
	
	if (0 != (error = device_data_start(mDevice))) {
		LOG3("failed to device_data_start, %d", error);
		[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Stopped" object:nil];
	}
	else
		[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Started" object:nil];
}

/**
 *
 *
 */
- (void)doHandleAD6620Stop:(NSNotification *)notification
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	
	int error;
	
	if (0 != (error = device_data_stop(mDevice))) {
		LOG3("failed to device_data_stop, %d", error);
		[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Started" object:nil];
	}
	else
		[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Stopped" object:nil];
}

/**
 *
 *
 */
- (void)doHandleAD6620Params:(NSNotification *)notification
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	
	int error;
	ad6620_t ad6620;
	NSDictionary *info = [notification userInfo];
	
	ad6620_init(&ad6620, NULL);
	
	for (NSString *key in info) {
		NSLog(@"%@ = %@", key, [info objectForKey:key]);
	}
	
	ad6620.params.scic2 = (uint8_t)[[info objectForKey:@"scic2"] intValue];
	ad6620.params.scic5 = (uint8_t)[[info objectForKey:@"scic5"] intValue];
	ad6620.params.mcic2 = (uint8_t)[[info objectForKey:@"mcic2"] intValue];
	ad6620.params.mcic5 = (uint8_t)[[info objectForKey:@"mcic5"] intValue];
	ad6620.params.mrcf  = (uint8_t)[[info objectForKey:@"rcf"  ] intValue];
	ad6620.params.sout  = 4;
	
	ad6620_stdcoeffs(&ad6620, [[info objectForKey:@"span"] unsignedIntValue]);
	
	if (0 != (error = device_program(mDevice, ad6620_chip(&ad6620))))
		LOG3("failed to device_program, %d", error);
	
	ad6620_destroy(&ad6620);
}

/**
 *
 *
 */
- (void)doHandleTerminate:(NSNotification *)notification
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	
	[self stop];
	device_disconnect(mDevice);
}





#pragma mark -
#pragma mark Other

/**
 *
 *
 */
- (void)setup
{
	int error;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	signal(SIGINT, sighandler);
	
	sleep(1);
	
	if (NULL == (mDevice = core_device_get(0)))
		LOG_ERROR_AND_GOTO(-101, done, "failed to core_device_get(0)");
	
	if (0 != (error = device_connect(mDevice)))
		LOG_ERROR_AND_GOTO(-102, done, "failed to device_connect(%s), %d", mDevice->desc.name, error);
	
	mDeviceController.span = 190;
	mDeviceController.gain = -10;
	mDeviceController.frequency = 1040000;
	mDeviceController.delegate = self;
	mDeviceController.context = mDevice;
	[mGraphController performSelectorOnMainThread:@selector(setLabel:) withObject:@"190 KHz span, -10 dB gain, 1040000 Hz" waitUntilDone:FALSE];
	
	if (0 != (error = eventmngr_handler_add2(core_eventmngr(), &mHandler, self, mDevice, EVENT_DEVICE, event_callback)))
		LOG_ERROR_AND_GOTO(-103, done, "failed to eventmngr_handler_add2(), %d", error);
	
	{
		//
		// overview graph
		//
		{
			graphplanar_t *graphplanar = NULL;
			dspchain_t *dspchain = NULL;
			datastream_t *datastream = NULL;
			
			short2double_t *short2double = NULL;
//		cpx2real_t *cpx2real = NULL;
//		chebyshev_t *chebyshev = NULL;
//		baseband_t *baseband = NULL;
			hamming_t *hamming = NULL;
			applefft_t *applefft = NULL;
//		dsptrim_t *trim = NULL;
			cpx2pwr_t *cpx2pwr = NULL;
			dspsmush_t *smush = NULL;
			average_t *average = NULL;
			smooth_t *smooth = NULL;
			
			if (0 != (error = core_graph_planar(&graphplanar, "Overview", 131072) || graphplanar == NULL))
				LOG_ERROR_AND_GOTO(-401, done, "failed to core_graph_planar(), %d", error);
			
			graphplanar_mode_set(graphplanar, GRAPH_PLANAR_MODE_CUR);
			
			mPlanar2 = graphplanar;
			
			graph_set_axis(graphplanar_graph(graphplanar), GRAPH_AXIS_X, "Frequency", GRAPH_SCALE_LIN, 1024., 0., (1040000.+(95000./2.)), (1040000-(95000./2.)), 1024);
			graph_set_axis(graphplanar_graph(graphplanar), GRAPH_AXIS_Y, "Amplitude", GRAPH_SCALE_LIN, 100., 0., 0., -100., (uint32_t)[mGraphController.planarview2 frame].size.height);
			graph_ready(graphplanar_graph(graphplanar));
			graph_datastream(graphplanar_graph(graphplanar), &datastream);
			graph_dspchain(graphplanar_graph(graphplanar), &dspchain);
			device_datastream_add(mDevice, datastream);
			
			core_dsp_type_short2double(&short2double);
//		core_dsp_other_cpx2real(&cpx2real);
//		core_dsp_filter_chebyshev(&chebyshev, 0.4, CHEBYSHEV_HIGH_PASS, 0.5, 6, 2048);
//		core_dsp_other_baseband(&baseband, 95000);
			core_dsp_window_hamming(&hamming, ASCP_IQ_COUNT, iqoffset);
			core_dsp_fft_applefft(&applefft, ASCP_IQ_COUNT, FFT_DIR_FORWARD, FFT_COMPLEX);
//		core_dsp_other_trim(&trim, sizeof(double) * ASCP_IQ_COUNT/2, 0);
			core_dsp_other_cpx2pwr(&cpx2pwr, 0., -100.);
			core_dsp_other_smush(&smush, ASCP_IQ_COUNT/4);
			core_dsp_other_average(&average, ASCP_IQ_COUNT/4, 5);
			core_dsp_other_smooth(&smooth, 2);
			
			dspchain_add(dspchain, (dsp_t*)short2double, DSPCHAIN_NEXT_POS);
//		dspchain_add(dspchain, (dsp_t*)cpx2real, DSPCHAIN_NEXT_POS);
//		dspchain_add(dspchain, (dsp_t*)chebyshev, DSPCHAIN_NEXT_POS);
//		dspchain_add(dspchain, (dsp_t*)baseband, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)hamming, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)applefft, DSPCHAIN_NEXT_POS);
//		dspchain_add(dspchain, (dsp_t*)trim, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)cpx2pwr, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)smush, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)average, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)smooth, DSPCHAIN_NEXT_POS);
			
			mGraphController.graphplanar2 = graphplanar;
			[mGraphController.planarview2 setDevice:mDevice];
			mGraphController.planarview2.tracking = FALSE;
			mGraphController.planarview2.dspchain = dspchain;
//		mGraphController.planarview2.dsptrim = trim;
//		mGraphController.planarview2.trimSize = sizeof(double) * ASCP_IQ_COUNT * 2;
			mGraphController.planarview2.graphController = mGraphController;
		}
		
		//
		// history graph
		//
		{
			graphhistory_t *graphhistory = NULL;
			dspchain_t *dspchain = NULL;
			datastream_t *datastream = NULL;
			
			short2double_t *short2double = NULL;
			hamming_t *hamming = NULL;
			applefft_t *applefft = NULL;
			dsptrim_t *trim = NULL;
			average_t *average = NULL;
			cpx2pwr_t *cpx2pwr = NULL;
			
			if (0 != (error = core_graph_history(&graphhistory, "History", 131072) || graphhistory == NULL))
				LOG_ERROR_AND_GOTO(-201, done, "failed to core_graph_history(), %d", error);
			
			mHistory = graphhistory;
			
			graph_set_axis(graphhistory_graph(graphhistory), GRAPH_AXIS_X, "Frequency", GRAPH_SCALE_LIN, 0., 0., 0., 0., 1024);
			graph_set_axis(graphhistory_graph(graphhistory), GRAPH_AXIS_Y, "Time", GRAPH_SCALE_LIN, 15., 0., 0., 0., (uint32_t)[mGraphController.historyview frame].size.height);
			graph_ready(graphhistory_graph(graphhistory));
			graph_datastream(graphhistory_graph(graphhistory), &datastream);
			graph_dspchain(graphhistory_graph(graphhistory), &dspchain);
			device_datastream_add(mDevice, datastream);
			
			core_dsp_type_short2double(&short2double);
			core_dsp_window_hamming(&hamming, ASCP_IQ_COUNT, iqoffset);
			core_dsp_fft_applefft(&applefft, ASCP_IQ_COUNT*4, FFT_DIR_FORWARD, FFT_COMPLEX);
			core_dsp_other_trim(&trim, sizeof(double) * ASCP_IQ_COUNT/2, 0);
			core_dsp_other_cpx2pwr(&cpx2pwr, 0., -100.);
			core_dsp_other_average(&average, ASCP_IQ_COUNT/4, 5);
			
			dspchain_add(dspchain, (dsp_t*)short2double, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)hamming, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)applefft, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)trim, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)cpx2pwr, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)average, DSPCHAIN_NEXT_POS);
			
			mGraphController.graphhistory = graphhistory;
			[mGraphController.historyview setDevice:mDevice];
			mGraphController.historyview.dspchain = dspchain;
			mGraphController.historyview.dsptrim = trim;
			mGraphController.historyview.trimSize = sizeof(double) * ASCP_IQ_COUNT * 2;
		}
		
		//
		// detail graph
		//
		{
			graphplanar_t *graphplanar = NULL;
			dspchain_t *dspchain = NULL;
			datastream_t *datastream = NULL;
			
			short2double_t *short2double = NULL;
			hamming_t *hamming = NULL;
			applefft_t *applefft = NULL;
			dsptrim_t *trim = NULL;
			average_t *average = NULL;
			cpx2pwr_t *cpx2pwr = NULL;
			smooth_t *smooth = NULL;
//		dspfileout_t *fileout = NULL;
			
			if (0 != (error = core_graph_planar(&graphplanar, "Planar", 131072) || graphplanar == NULL))
				LOG_ERROR_AND_GOTO(-301, done, "failed to core_graph_planar(), %d", error);
			
			graphplanar_mode_set(graphplanar, GRAPH_PLANAR_MODE_AVG | GRAPH_PLANAR_MODE_CUR | GRAPH_PLANAR_MODE_MAX);
			
			mPlanar = graphplanar;
			
			graph_set_axis(graphplanar_graph(graphplanar), GRAPH_AXIS_X, "Frequency", GRAPH_SCALE_LIN, 1024., 0., (1040000.+(95000./2.)), (1040000-(95000./2.)), 1024);
			graph_set_axis(graphplanar_graph(graphplanar), GRAPH_AXIS_Y, "Amplitude", GRAPH_SCALE_LIN, 100., 0., 0., -100., (uint32_t)[mGraphController.planarview frame].size.height);
			graph_ready(graphplanar_graph(graphplanar));
			graph_datastream(graphplanar_graph(graphplanar), &datastream);
			graph_dspchain(graphplanar_graph(graphplanar), &dspchain);
			device_datastream_add(mDevice, datastream);
			
			core_dsp_type_short2double(&short2double);
			core_dsp_window_hamming(&hamming, ASCP_IQ_COUNT, iqoffset);
			core_dsp_fft_applefft(&applefft, ASCP_IQ_COUNT*4, FFT_DIR_FORWARD, FFT_COMPLEX);
			core_dsp_other_trim(&trim, sizeof(double) * ASCP_IQ_COUNT/2, 0);
			core_dsp_other_cpx2pwr(&cpx2pwr, 0., -100.);
			core_dsp_other_average(&average, ASCP_IQ_COUNT/4, 5);
			core_dsp_other_smooth(&smooth, 2);
//		core_dsp_other_fileout(&fileout, "/Users/cjones/Desktop/static.txt");
			
			dspchain_add(dspchain, (dsp_t*)short2double, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)hamming, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)applefft, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)trim, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)cpx2pwr, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)average, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)smooth, DSPCHAIN_NEXT_POS);
//		dspchain_add(dspchain, (dsp_t*)fileout, DSPCHAIN_NEXT_POS);
			
			mGraphController.graphplanar = graphplanar;
			[mGraphController.planarview setDevice:mDevice];
			mGraphController.planarview.dspchain = dspchain;
			mGraphController.planarview.dsptrim = trim;
			mGraphController.planarview.trimSize = sizeof(double) * ASCP_IQ_COUNT * 2;
		}
		
		//
		// sound
		//
		// convert quadrature data for filtering (real, complex, power, absolute?)
		// add high-pass filter
		// add low-pass filter
		// down sample to 44.1KHz
		// send to core audio buffer
		//
		// vfo = 700 000 Hz
		// fc = 196 000 Hz
		// bw = 190 000 Hz
		// range = +- 85 000 Hz (615 000 Hz - 785 000 Hz)
		// target = 680 000 Hz
		// hi = 675Khz = 0.255
		// lo = 685Khz = 
		//
		/*
		{
			coreaudio_t *audio = malloc(sizeof(coreaudio_t));
			dspchain_t *dspchain = NULL;
			datastream_t *datastream = NULL;
			
			chebyshev_t *chebyshev_hi = NULL;
			chebyshev_t *chebyshev_lo = NULL;
			
			if (0 != (error = coreaudio_init(audio, sizeof(double)*4096, NULL)))
				LOG_ERROR_AND_GOTO(-401, done, "failed to coreaudio_init(), %d", error);
			
			audio_datastream(coreaudio_audio(audio), &datastream);
			audio_dspchain(coreaudio_audio(audio), &dspchain);
//		device_datastream_add(mDevice, datastream);
			
			core_dsp_filter_chebyshev(&chebyshev_hi, 0.45, CHEBYSHEV_LOW_PASS, 0.5, 6, 100);
			core_dsp_filter_chebyshev(&chebyshev_lo, 0.45, CHEBYSHEV_HIGH_PASS, 0.5, 6, 100);
			
			dspchain_add(dspchain, (dsp_t*)chebyshev_hi, DSPCHAIN_NEXT_POS);
			dspchain_add(dspchain, (dsp_t*)chebyshev_lo, DSPCHAIN_NEXT_POS);
		}
		*/
		
	}
	
	// create an objective-c wrapper for a device_t
	// add all of the graphs to an array in the wrapper class
	//
	// the problem with this method is that it is going to take a ton of time to update the various 
	// graphs and what not, and execution is taking place on the device read thread.
	
	mDevice->span = 190;
	mDevice->gain = -10;
	mDevice->frequency = 1040000;
	
	gDevice = mDevice;
	
	[mGraphController performSelectorOnMainThread:@selector(redraw) withObject:nil waitUntilDone:FALSE];
	
	[self performSelectorOnMainThread:@selector(doActionInfo:) withObject:nil waitUntilDone:FALSE];
	
done:
	[pool release];
}





#pragma mark -
#pragma mark DeviceProtocol

/**
 * Called when the user indicates he wants to change the frequency. We assume that the given
 * frequency is the center frequency and that we have a 190 KHz span.
 *
 */
- (void)deviceFrequency:(NSInteger)frequency context:(void *)context
{
	device_t *device = (device_t*)context;
	NSInteger frequencyHi, frequencyLo;
	
	frequencyHi = frequency + (190000 / 2);
	frequencyLo = frequency - (190000 / 2);
	
	NSLog(@"frequency = %ld, high = %ld, low = %ld", frequency, frequencyHi, frequencyLo);
	
	device_frequency_set(device, (uint32_t)frequency);
	
	// TODO: move this stuff to the graph controller
	
	mPlanar->graph.axis_x.unit_high = frequencyHi;
	mPlanar->graph.axis_x.unit_low  = frequencyLo;
	
	mPlanar2->graph.axis_x.unit_high = frequencyHi;
	mPlanar2->graph.axis_x.unit_low  = frequencyLo;
	
	mGraphController.planarview2.selectBeg = 0;
	mGraphController.planarview2.selectEnd = 128;
	
	[mGraphController setFrequencyHigh:frequencyHi low:frequencyLo];
	[mDeviceController setFrequency:frequency];
	
	[mGraphController setLabel:[NSString stringWithFormat:@"%d KHz span, %d dB gain, %lu Hz", device->span, (int)device->gain, device->frequency]];
}

/**
 *
 *
 */
- (void)deviceGain:(NSInteger)gain context:(void *)context
{
	device_t *device = (device_t*)context;
	
	NSLog(@"gain = %ld", gain);
	
	device_gain_set(device, (int32_t)gain);
	
	[mGraphController reset];
	[mGraphController setLabel:[NSString stringWithFormat:@"%d KHz span, %d dB gain, %lu Hz", device->span, (int)device->gain, device->frequency]];
}

/**
 *
 *
 */
- (void)deviceSpan:(NSInteger)span context:(void *)context
{
	uint32_t data_on = 0;
	device_t *device = (device_t*)context;
	
	NSLog(@"span = %ld", span);
	
	if ((data_on = device->data_on))
		device_data_stop(device);
	
	device_span_set(device, (uint32_t)span);
	[mGraphController reset];
	
	if (data_on)
		device_data_start(device);
	
	[mGraphController setLabel:[NSString stringWithFormat:@"%d KHz span, %d dB gain, %lu Hz", device->span, (int)device->gain, device->frequency]];
}

/**
 *
 *
 */
- (void)deviceUnit:(NSInteger)unit context:(void *)context
{
	//device_t *device = (device_t*)context;
	
	NSLog(@"unit = %ld", unit);
}

@end
