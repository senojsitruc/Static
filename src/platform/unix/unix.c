/*
 *  unix.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.13.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "unix.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../core/core.h"
#include "../../device/device.h"
#include "../../dsp/fft/ooura4/ooura4.h"
#include "../../dsp/other/average/average.h"
#include "../../dsp/other/cpx2pwr/cpx2pwr.h"
#include "../../dsp/other/smooth/smooth.h"
#include "../../dsp/window/hamming/hamming.h"
#include "../../misc/mem/memlock.h"
#include "../../misc/logger.h"
#include "../../output/graph/graph.h"
#include "../../output/graph/history/history.h"
#include "../../output/graph/planar/planar.h"
#include "../../protocol/ascp/ascp.h"

device_t *gDevice;
double iqoffset;

/**
 *
 *
 */
static void
sighandler (int sig)
{
	if (gDevice != NULL) {
		device_disconnect(gDevice);
		gDevice = NULL;
	}
}

/**
 *
 *
 */
static
double calc_offset_iq ()
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
int
graph_draw_path (graph_t *graph, void *context, int fill, float *points, uint32_t count, float r, float g, float b, float a)
{
	
	return 0;
}

/**
 *
 *
 */
int
graph_draw_line (graph_t *graph, void *context, float x1, float y1, float x2, float y2, float r, float g, float b, float a)
{
	
	return 0;
}

/**
 *
 *
 */
int
graph_draw_hist (graph_t *graph, void *context, uint32_t *data, uint32_t width, uint32_t height)
{
	//LOG3("%s..");
	
	return 0;
}

/**
 *
 *
 */
int
graph_redraw (graph_t *graph, void *context)
{
	return graph_draw(graph, context);
}

/**
 *
 *
 */
int
main (int argc, char **argv)
{
	int error;
	device_t *device;
	
	iqoffset = calc_offset_iq();
	
	signal(SIGINT, sighandler);
	
	if (0 != (error = memlock_setup()))
		LOG_ERROR_AND_RETURN(EXIT_FAILURE, "failed to memlock_init(), %d", error);
	
	if (0 != (error = core_init()))
		LOG_ERROR_AND_RETURN(EXIT_FAILURE, "failed to core_init(), %d", error);
	
	sleep(1);
	
	if (NULL == (gDevice = device = core_device_get(0)))
		LOG_ERROR_AND_RETURN(EXIT_FAILURE, "failed to core_device_get(0)");
	
	if (0 != (error = device_connect(device)))
		LOG_ERROR_AND_RETURN(EXIT_FAILURE, "failed to device_connect(), %d", error);
	
	device->span = 190;
	device->gain = -10;
	device->frequency = 1040000;
	
	//
	// planar graph
	//
	{
		graphplanar_t *graphplanar = NULL;
		dspchain_t *dspchain = NULL;
		hamming_t *hamming = NULL;
		ooura4_t *ooura4 = NULL;
		average_t *average = NULL;
		cpx2pwr_t *cpx2pwr = NULL;
		smooth_t *smooth = NULL;
		invert_t *invert = NULL;
		datastream_t *datastream = NULL;
		
		if (0 != (error = core_graph_planar(&graphplanar, "Planar") || graphplanar == NULL))
			LOG_ERROR_AND_RETURN(EXIT_FAILURE, "failed to core_graph_planar(), %d", error);
		
		graph_set_axis(graphplanar_graph(graphplanar), GRAPH_AXIS_X, "Frequency", GRAPH_SCALE_LIN, 0., 0., 1024);
		graph_set_axis(graphplanar_graph(graphplanar), GRAPH_AXIS_Y, "Amplitude", GRAPH_SCALE_LIN, 100., 0., 250);
		graph_ready(graphplanar_graph(graphplanar));
		graph_datastream(graphplanar_graph(graphplanar), &datastream);
		graph_dspchain(graphplanar_graph(graphplanar), &dspchain);
		device_datastream_add(device, datastream);
		
		core_dsp_window_hamming(&hamming, ASCP_IQ_COUNT, iqoffset);
		core_dsp_fft_ooura4(&ooura4, ASCP_IQ_COUNT, FFT_DIR_FORWARD);
		core_dsp_other_cpx2pwr(&cpx2pwr, 0., -100.);
		core_dsp_other_average(&average, ASCP_IQ_COUNT/4, 100);
		core_dsp_other_invert(&invert, INVERT_REAL);
		core_dsp_other_smooth(&smooth, 3);
		
		dspchain_add(dspchain, (dsp_t*)hamming, -1);
		dspchain_add(dspchain, (dsp_t*)ooura4, -1);
		dspchain_add(dspchain, (dsp_t*)cpx2pwr, -1);
		dspchain_add(dspchain, (dsp_t*)average, -1);
		dspchain_add(dspchain, (dsp_t*)invert, -1);
		dspchain_add(dspchain, (dsp_t*)smooth, -1);
		
		graph_set_redraw_fp(graphplanar_graph(graphplanar), (graph_redraw_fp_func)graph_redraw, graphplanar);
		graph_set_draw_path_fp(graphplanar_graph(graphplanar), (graph_draw_path_fp_func)graph_draw_path);
		graph_set_draw_line_fp(graphplanar_graph(graphplanar), (graph_draw_line_fp_func)graph_draw_line);
	}
	
	//
	// history graph
	//
	{
		graphhistory_t *graphhistory = NULL;
		dspchain_t *dspchain = NULL;
		hamming_t *hamming = NULL;
		ooura4_t *ooura4 = NULL;
		average_t *average = NULL;
		cpx2pwr_t *cpx2pwr = NULL;
		invert_t *invert = NULL;
		datastream_t *datastream = NULL;
		
		if (0 != (error = core_graph_history(&graphhistory, "History") || graphhistory == NULL))
			LOG_ERROR_AND_RETURN(EXIT_FAILURE, "failed to core_graph_history(), %d", error);
		
		graph_set_axis(graphhistory_graph(graphhistory), GRAPH_AXIS_X, "Frequency", GRAPH_SCALE_LIN, 0., 0., 1024);
		graph_set_axis(graphhistory_graph(graphhistory), GRAPH_AXIS_Y, "Time", GRAPH_SCALE_LIN, 15., 0., 250);
		graph_ready(graphhistory_graph(graphhistory));
		graph_datastream(graphhistory_graph(graphhistory), &datastream);
		graph_dspchain(graphhistory_graph(graphhistory), &dspchain);
		device_datastream_add(device, datastream);
		
		core_dsp_window_hamming(&hamming, ASCP_IQ_COUNT, iqoffset);
		core_dsp_fft_ooura4(&ooura4, ASCP_IQ_COUNT, FFT_DIR_FORWARD);
		core_dsp_other_cpx2pwr(&cpx2pwr, 0., -100.);
		core_dsp_other_invert(&invert, INVERT_REAL);
		core_dsp_other_average(&average, ASCP_IQ_COUNT/4, 100);
		
		dspchain_add(dspchain, (dsp_t*)hamming, -1);
		dspchain_add(dspchain, (dsp_t*)ooura4, -1);
		dspchain_add(dspchain, (dsp_t*)cpx2pwr, -1);
		dspchain_add(dspchain, (dsp_t*)invert, -1);
		dspchain_add(dspchain, (dsp_t*)average, -1);
		
		graph_set_redraw_fp(graphhistory_graph(graphhistory), (graph_redraw_fp_func)graph_redraw, graphhistory);
		graph_set_draw_hist_fp(graphhistory_graph(graphhistory), (graph_draw_hist_fp_func)graph_draw_hist);
		graph_set_draw_path_fp(graphhistory_graph(graphhistory), (graph_draw_path_fp_func)graph_draw_path);
		graph_set_draw_line_fp(graphhistory_graph(graphhistory), (graph_draw_line_fp_func)graph_draw_line);
	}
	
	sleep(1);
	
	if (0 != (error = device_span_set(device, device->span)))
		LOG_ERROR_AND_RETURN(EXIT_FAILURE, "failed to device_span_set(), %d", error);
	
	sleep(1);
	
	if (0 != (error = device_data_start(device)))
		LOG_ERROR_AND_RETURN(EXIT_FAILURE, "failed to device_data_start(), %d", error);
	
	while (1)
		sleep(1);
	
	return EXIT_SUCCESS;
}
