//
//  DetailGraphView.m
//  Static
//
//  Created by Curtis Jones on 2010.11.27.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "DetailGraphView.h"
#import "../../core/core.h"
#import "../../misc/logger.h"
#import "../../dsp/fft/applefft/applefft.h"
#import "../../dsp/other/average/average.h"
#import "../../dsp/other/baseband/baseband.h"
#import "../../dsp/other/cpx2pwr/cpx2pwr.h"
#import "../../dsp/other/cpx2real/cpx2real.h"
#import "../../dsp/other/fileout/fileout.h"
#import "../../dsp/other/trim/trim.h"
#import "../../dsp/other/scale/double.h"
#import "../../dsp/other/scale/scale.h"
#import "../../dsp/other/smooth/smooth.h"
#import "../../dsp/other/zero/zero.h"
#import "../../dsp/type/double2sint16/double2sint16.h"
#import "../../dsp/type/short2double/short2double.h"
#import "../../output/audio/coreaudio/coreaudio.h"
#import "../../protocol/ascp/ascp.h"
#import <errno.h>
#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <sys/time.h>

#define MAX_ALPHA .8
#define LINE_LEN 11.

@implementation DetailGraphView





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
- (id)initWithFrame:(NSRect)frame
{
	NSLog(@"%s.. x=%f, y=%f, width=%f, height=%f", __PRETTY_FUNCTION__, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
	
	self = [super initWithFrame:frame];
	
	if (self) {
		mSelectMenu = [[NSMenu alloc] initWithTitle:@"Selection Menu"];
		[mSelectMenu insertItemWithTitle:@"Demodulate AM (v2)" action:@selector(doActionDemodulateTwo:) keyEquivalent:@"" atIndex:0];
		[mSelectMenu insertItemWithTitle:@"Demodulate AM (v1)" action:@selector(doActionDemodulateOne:) keyEquivalent:@"" atIndex:0];
	}
	
	return self;
}

/**
 *
 *
 */
- (void)dealloc
{
	[mSelectMenu release];
	[super dealloc];
}





#pragma mark -
#pragma mark NSView

/**
 *
 *
 */
- (void)drawRect:(NSRect)dirtyRect
{
	if (mGraph == NULL)
		return;
	
	CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
	NSRect frame = [self frame];
	NSPoint point = [self convertPoint:[[self window] mouseLocationOutsideOfEventStream] fromView:nil];
	BOOL mouseInside = FALSE;
	
	if (point.x > 0. && point.y > 0. && point.x < frame.size.width && point.y < frame.size.height)
		mouseInside = TRUE;
	
	mGraph->graph.axis_y.length = (uint32_t)[self frame].size.height;
	graph_draw(graphplanar_graph(mGraph), context);
	
	mIsDirty = FALSE;
	
	// draw the selected range
	if (mSelect) {
		size_t text_l;
		char text[100];
		CGFloat selectLo, selectHi;
		NSUInteger freqLo, freqHi;
		
		if (mSelectBeg < mSelectEnd) {
			selectLo = mSelectBeg;
			selectHi = mSelectEnd;
			freqLo = mFrequencyBeg;
			freqHi = mFrequencyEnd;
		}
		else {
			selectLo = mSelectEnd;
			selectHi = mSelectBeg;
			freqLo = mFrequencyEnd;
			freqHi = mFrequencyBeg;
		}
		
		// the box
		{
			CGContextBeginPath(context);
			CGContextSetLineWidth(context, 1.);
			CGContextSetRGBStrokeColor(context, 1., 1., 0., MAX_ALPHA);
			CGContextSetRGBFillColor(context, .19, .8, .27, 0.65);
			
			CGContextMoveToPoint(context, mSelectBeg, 2.);
			CGContextAddLineToPoint(context, mSelectBeg, frame.size.height-3);
			CGContextAddLineToPoint(context, mSelectEnd, frame.size.height-3);
			CGContextAddLineToPoint(context, mSelectEnd, 2.);
			CGContextClosePath(context);
			
			CGContextDrawPath(context, kCGPathFillStroke);
		}
		
		if (mouseInside == TRUE) {
			// left label (the low value)
			{
				text_l = (size_t)snprintf(text, 100, "%lf MHz", freqLo/1000000.);
				
				CGContextSelectFont(context, "Helvetica", 9., kCGEncodingMacRoman);
				CGContextSetCharacterSpacing(context, 2);
				CGContextSetTextDrawingMode(context, kCGTextFillStroke);
				CGContextSetLineWidth(context, 1.);
				CGContextSetRGBStrokeColor(context, 1., 1., 0., MAX_ALPHA);
				
				CGPoint textpos1 = CGContextGetTextPosition(context);
				CGContextSetTextDrawingMode(context, kCGTextInvisible);
				CGContextShowText(context, text, text_l);
				CGPoint textpos2 = CGContextGetTextPosition(context);
				int textw = (int)(textpos2.x - textpos1.x);
				
				CGContextSetTextDrawingMode(context, kCGTextFillStroke);
				
				CGContextSaveGState(context);
				CGContextTranslateCTM(context, selectLo-5., (frame.size.height/2.)-(textw/2.));
				CGContextRotateCTM(context, (M_PI/2.));
				CGContextShowTextAtPoint(context, 0., 0., text, text_l);
				CGContextRestoreGState(context);
			}
			
			// right label (the high value)
			{
				text_l = (size_t)snprintf(text, 100, "%lf MHz", freqHi/1000000.);
				
				CGContextSelectFont(context, "Helvetica", 9., kCGEncodingMacRoman);
				CGContextSetCharacterSpacing(context, 2);
				CGContextSetTextDrawingMode(context, kCGTextFillStroke);
				CGContextSetLineWidth(context, 1.);
				CGContextSetRGBStrokeColor(context, 1., 1., 0., MAX_ALPHA);
				
				CGPoint textpos1 = CGContextGetTextPosition(context);
				CGContextSetTextDrawingMode(context, kCGTextInvisible);
				CGContextShowText(context, text, text_l);
				CGPoint textpos2 = CGContextGetTextPosition(context);
				int textw = (int)(textpos2.x - textpos1.x);
				
				CGContextSetTextDrawingMode(context, kCGTextFillStroke);
				
				CGContextSaveGState(context);
				CGContextTranslateCTM(context, selectHi+12., (frame.size.height/2.)-(textw/2.));
				CGContextRotateCTM(context, (M_PI/2.));
				CGContextShowTextAtPoint(context, 0., 0., text, text_l);
				CGContextRestoreGState(context);
			}
		}
	}
	
	[super drawRect:dirtyRect];
}





#pragma mark -
#pragma mark NSResonder

/**
 *
 *
 */
- (void)mouseDown:(NSEvent *)theEvent
{
	int error;
	graph_value_t value;
	NSPoint event_location = [theEvent locationInWindow];
	NSPoint local_point = [self convertPoint:event_location fromView:nil];
	NSUInteger flags = [theEvent modifierFlags];
	
	if (local_point.x >= MIN(mSelectBeg,mSelectEnd) && local_point.x <= MAX(mSelectBeg,mSelectEnd))
		mInsideSelection = TRUE;
	else
		mInsideSelection = FALSE;
	
	if (!mInsideSelection) {
		if (NSAlternateKeyMask & flags)
			mSelect = TRUE;
		else
			mSelect = FALSE;
	}
	
	if (unlikely(0 != (error = graph_get_value(graphplanar_graph(mGraph), (uint32_t)local_point.x, (uint32_t)local_point.y, &value))))
		LOG_ERROR_AND_RETURN(, "failed to graph_get_value, %d", error);
	
	mDragged = FALSE;
	mDownPt = local_point;
	mPrevPt = local_point;
	
	if (!mInsideSelection) {
		mFrequencyBeg = (NSUInteger)value.xaxis;
		mFrequencyEnd = (NSUInteger)value.xaxis;
		mSelectBeg = local_point.x;
		mSelectEnd = local_point.x;
	}
}

/**
 *
 *
 */
- (void)mouseUp:(NSEvent *)theEvent
{
	int error;
	graph_value_t value;
	
	[[self window] makeFirstResponder:self];
	
	if (theEvent.clickCount == 2) {
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		
		NSLog(@"x=%f, y=%f", local_point.x, local_point.y);
	}
	
	if (mDragged && !mInsideSelection) {
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		
		if (mDevice == NULL)
			return;
		
		if (unlikely(0 != (error = graph_get_value(graphplanar_graph(mGraph), (uint32_t)local_point.x, (uint32_t)local_point.y, &value))))
			LOG_ERROR_AND_RETURN(, "failed to graph_get_value, %d", error);
		
		double span = mGraph->graph.axis_x.unit_high - mGraph->graph.axis_x.unit_low;
		double center = mGraph->graph.axis_x.unit_low + (span/2.);
		double frequency = center + (mFrequencyBeg - value.xaxis);
		
		// if we're selecting a range, ...
		if (mSelect == TRUE) {
			NSLog(@"  range selected from %lu to %f", mFrequencyBeg, frequency);
		}
		
		// otherwise, we're shifting the frequency
		else {
			if (0 != (error = device_frequency_set(mDevice, (uint32_t)frequency)))
				LOG_ERROR_AND_RETURN(, "failed to device_frequency_set(%f), %d", frequency, error);
		}
	}
	
	// if the mouse came up and we didn't drag at all, then clear the selection
	else if (!mInsideSelection) {
		mSelect = FALSE;
		
		dspchain_t *dspchain = NULL;
		datastream_t *datastream = NULL;
		
		audio_datastream(coreaudio_audio(coreaudio), &datastream);
		audio_dspchain(coreaudio_audio(coreaudio), &dspchain);
		
		device_datastream_del(mDevice, datastream);
		
		coreaudio_release(coreaudio);
		coreaudio = NULL;
	}
	
	[self performSelectorOnMainThread:@selector(redraw) withObject:nil waitUntilDone:FALSE];
}

/**
 *
 *
 */
- (void)mouseDragged:(NSEvent *)theEvent
{
	int error;
	graph_value_t value;
	NSPoint event_location = [theEvent locationInWindow];
	NSPoint local_point = [self convertPoint:event_location fromView:nil];
	
	mDragged = TRUE;
	
	if (mInsideSelection) {
		CGFloat delta_x = local_point.x - mPrevPt.x;
		
		mSelectBeg += delta_x;
		mSelectEnd += delta_x;
		
		if (unlikely(0 != (error = graph_get_value(graphplanar_graph(mGraph), (uint32_t)mSelectBeg, (uint32_t)local_point.y, &value))))
			LOG_ERROR_AND_RETURN(, "failed to graph_get_value(), %d", error);
		
		mFrequencyBeg = (NSUInteger)value.xaxis;
		
		if (unlikely(0 != (error = graph_get_value(graphplanar_graph(mGraph), (uint32_t)mSelectBeg, (uint32_t)local_point.y, &value))))
			LOG_ERROR_AND_RETURN(, "failed to graph_get_value(), %d", error);
		
		mFrequencyEnd = (NSUInteger)value.xaxis;
		
		mPrevPt = local_point;
	}
	else {
		if (unlikely(0 != (error = graph_get_value(graphplanar_graph(mGraph), (uint32_t)local_point.x, (uint32_t)local_point.y, &value))))
			LOG_ERROR_AND_RETURN(, "failed to graph_get_value(), %d", error);
		
		mFrequencyEnd = (NSUInteger)value.xaxis;
		mSelectEnd = local_point.x;
	}
	
	[self performSelectorOnMainThread:@selector(redraw) withObject:nil waitUntilDone:FALSE];
}

/**
 *
 *
 */
- (void)mouseEntered:(NSEvent *)theEvent
{
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	
	if (mFade == 0) {
		mFade = (uint64_t)((tv.tv_sec*1000) + (tv.tv_usec/1000));
		mAlpha = 0.;
		[self performSelectorOnMainThread:@selector(redraw) withObject:nil waitUntilDone:FALSE];
	}
	
	mInside = TRUE;
}

/**
 *
 *
 */
- (void)mouseExited:(NSEvent *)theEvent
{
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	
	if (mFade == 0) {
		mFade = (uint64_t)((tv.tv_sec*1000) + (tv.tv_usec/1000));
		[self performSelectorOnMainThread:@selector(redraw) withObject:nil waitUntilDone:FALSE];
	}
	
	mInside = FALSE;
}

/**
 *
 *
 */
- (void)mouseMoved:(NSEvent *)theEvent
{
	[self performSelectorOnMainThread:@selector(redraw) withObject:nil waitUntilDone:FALSE];
}

/**
 *
 *
 */
- (void)rightMouseDown:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	NSPoint local_point = [self convertPoint:event_location fromView:nil];
	
	if (mSelect) {
		CGFloat selectLo, selectHi;
		
		if (mSelectBeg < mSelectEnd) {
			selectLo = mSelectBeg;
			selectHi = mSelectEnd;
		}
		else {
			selectLo = mSelectEnd;
			selectHi = mSelectBeg;
		}
		
		if (local_point.x >= selectLo && local_point.x <= selectHi)
			[NSMenu popUpContextMenu:mSelectMenu withEvent:theEvent forView:self];
	}
}





#pragma mark -
#pragma mark Actions

/**
 *
 *
 */
- (void)doActionDemodulateOne:(NSNotification *)notification
{
	int error;
	NSUInteger passBeg, passEnd, passTmp;
	
	passBeg = (mDspTrim->offset / 4) + ((NSUInteger)mSelectBeg / 4 * 16);
	passEnd = (mDspTrim->offset / 4) + ((NSUInteger)mSelectEnd / 4 * 16);
	
	passBeg -= (passBeg % 16);
	passEnd += 16 - (passEnd % 16);
	
	// if the user selected the range right-to-left then the begin point is greater than the end
	// point and we want to ensure that we beg < end, so swap the beg/end values
	if (passBeg > passEnd) {
		passTmp = passBeg;
		passBeg = passEnd;
		passEnd = passTmp;
	}
	
	NSLog(@"  demodulate [offset=%u, selectBeg=%f, selectEnd=%f, passBeg=%lu, passEnd=%lu", mDspTrim->offset, mSelectBeg, mSelectEnd, passBeg, passEnd);
	
	{
		dspchain_t *dspchain = NULL;
		datastream_t *datastream = NULL;
		
		short2double_t *short2double = NULL;
		applefft_t *applefft_t2f = NULL;
		dspshift_t *dspshift = NULL;
		dspzero_t *dspzero = NULL;
		applefft_t *applefft_f2t = NULL;
		cpx2real_t *cpx2real = NULL;
		scaledouble_t *scaledouble = NULL;
//	dspfileout_t *fileout = NULL;
		double2sint16_t *dbl2sint16 = NULL;
		
		if (0 != (error = core_audio_coreaudio(&coreaudio, 131072) || coreaudio == NULL))
			LOG_ERROR_AND_RETURN(, "failed to core_audio_coreaudio(), %d", error);
		
		audio_datastream(coreaudio_audio(coreaudio), &datastream);
		audio_dspchain(coreaudio_audio(coreaudio), &dspchain);
		
		core_dsp_type_short2double(&short2double);
		core_dsp_fft_applefft(&applefft_t2f, ASCP_IQ_COUNT, FFT_DIR_FORWARD, FFT_COMPLEX);
		core_dsp_other_shift(&dspshift, DSPSHIFT_DIR_LEFT, (uint32_t)(passBeg-16));
		core_dsp_other_zero(&dspzero, DSPSCALE_MODE_LIN, (uint32_t)16, (uint32_t)passEnd+16);
//	core_dsp_other_zero(&dspzero, DSPSCALE_MODE_LIN, (uint32_t)passBeg, (uint32_t)(passEnd - passBeg));
		core_dsp_fft_applefft(&applefft_f2t, ASCP_IQ_COUNT, FFT_DIR_BACKWARD, FFT_COMPLEX);
		core_dsp_other_cpx2real(&cpx2real);
		core_dsp_other_scale_double(&scaledouble, DSPSCALE_MODE_LIN, 32768., -32767., 32768., 0.);
		core_dsp_type_double2sint16(&dbl2sint16);
//	core_dsp_other_fileout(&fileout, DSP_DATA_TYPE_SINT16, "/Users/cjones/Desktop/static.txt");
		
		dspchain_add(dspchain, (dsp_t*)short2double, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)applefft_t2f, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)dspshift, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)dspzero, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)applefft_f2t, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)cpx2real, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)scaledouble, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)dbl2sint16, DSPCHAIN_NEXT_POS);
//	dspchain_add(dspchain, (dsp_t*)fileout, DSPCHAIN_NEXT_POS);
		
		device_datastream_add(mDevice, datastream);
	}
	
}

/**
 *
 *
 */
- (void)doActionDemodulateTwo:(NSNotification *)notification
{
	int error;
	NSUInteger passBeg, passEnd, passTmp, frequency;
	
	passBeg = (mDspTrim->offset / 4) + ((NSUInteger)mSelectBeg / 4 * 16);
	passEnd = (mDspTrim->offset / 4) + ((NSUInteger)mSelectEnd / 4 * 16);
	
	// adjust to the low value down to a 16 byte boundary and adjust the high value up to a 16 byte
	// boundary.
	passBeg -= (passBeg % 16);
	passEnd += 16 - (passEnd % 16);
	
	// if the user selected the range right-to-left then the begin point is greater than the end
	// point and we want to ensure that we beg < end, so swap the beg/end values
	if (passBeg > passEnd) {
		passTmp = passBeg;
		passBeg = passEnd;
		passEnd = passTmp;
	}
	
	frequency = (NSUInteger)(190000. * (((double)passBeg / 16.) / 2048.));
	
	NSLog(@"  demodulate [offset=%u, selectBeg=%f, selectEnd=%f, passBeg=%lu, passEnd=%lu, frequency=%lu", mDspTrim->offset, mSelectBeg, mSelectEnd, passBeg, passEnd, frequency);
	
	{
		dspchain_t *dspchain = NULL;
		datastream_t *datastream = NULL;
		
		short2double_t *short2double = NULL;
		baseband_t *baseband = NULL;
		cpx2real_t *cpx2real = NULL;
		chebyshev_t *chebyshev = NULL;
		dspfileout_t  *fileout = NULL;
		scaledouble_t *scaledouble = NULL;
		double2sint16_t *dbl2sint16 = NULL;
		
		if (0 != (error = core_audio_coreaudio(&coreaudio, 131072) || coreaudio == NULL))
			LOG_ERROR_AND_RETURN(, "failed to core_audio_coreaudio(), %d", error);
		
		audio_datastream(coreaudio_audio(coreaudio), &datastream);
		audio_dspchain(coreaudio_audio(coreaudio), &dspchain);
		
		core_dsp_type_short2double(&short2double);
		core_dsp_other_baseband(&baseband, (uint32_t)frequency);
		core_dsp_other_cpx2real(&cpx2real);
		core_dsp_filter_chebyshev(&chebyshev, 0.4, CHEBYSHEV_LOW_PASS, 0.5, 6, 4096);
		core_dsp_other_fileout(&fileout, DSP_DATA_TYPE_DOUBLE, "/Users/cjones/Desktop/static.txt");
//	core_dsp_other_scale_double(&scaledouble, DSPSCALE_MODE_LIN, 32768., -32767., 32768., 0.);
		core_dsp_other_scale_double(&scaledouble, DSPSCALE_MODE_LIN, 1000., -1000., 32768., 0.);
		core_dsp_type_double2sint16(&dbl2sint16);
		
		dspchain_add(dspchain, (dsp_t*)short2double, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)baseband, DSPCHAIN_NEXT_POS);
//	dspchain_add(dspchain, (dsp_t*)cpx2real, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)chebyshev, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)cpx2real, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)fileout, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)scaledouble, DSPCHAIN_NEXT_POS);
		dspchain_add(dspchain, (dsp_t*)dbl2sint16, DSPCHAIN_NEXT_POS);
		
		device_datastream_add(mDevice, datastream);
	}
	
}

@end
