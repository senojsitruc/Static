//
//  PlanarGraphView.m
//  Static
//
//  Created by Curtis Jones on 2009.12.29.
//  Copyright 2009 Curtis Jones. All rights reserved.
//

#import "PlanarGraphView.h"
#import "../../misc/logger.h"
#import "GraphController.h"
#import <errno.h>
#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <sys/time.h>

#define MAX_ALPHA .8
#define LINE_LEN 11.





/**
 *
 *
 */
static int
__redraw (graph_t *graph, PlanarGraphView *pgv)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(pgv == NULL))
		LOG_ERROR_AND_RETURN(-2, "null PlanarGraphView");
	
	if (!pgv.isdirty)
		[pgv performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:FALSE];
	
	return 0;
}

/**
 *
 *
 */
static int
__draw_path (graphplanar_t *graph, CGContextRef context, int fill, float *points, uint32_t count, float r, float g, float b, float a)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	if (unlikely(context == NULL))
		LOG_ERROR_AND_RETURN(-2, "null CGContextRef");
	
	if (unlikely(points == NULL))
		LOG_ERROR_AND_RETURN(-3, "null points");
	
	if (count == 0)
		return 0;
	
	CGContextBeginPath(context);
	CGContextSetLineWidth(context, 1.);
	CGContextSetRGBStrokeColor(context, r, g, b, a);
	
	if (fill)
		CGContextSetRGBFillColor(context, r, g, b, a);
	
	CGContextMoveToPoint(context, 0., 0.);
	
	for (uint32_t i = 0; i < count; ++i, ++points) {
		float point = *points;
		
		if (point == 0.0)
			continue;
		
		CGContextAddLineToPoint(context, (float)i, point);
	}
	
	CGContextAddLineToPoint(context, count-1, 0.);
	CGContextClosePath(context);
	
	if (fill)
		CGContextDrawPath(context, kCGPathFill);
	else
		CGContextDrawPath(context, kCGPathStroke);
	
	return 0;
}

/**
 *
 *
 */
static int
__draw_line (graph_t *graph, CGContextRef context, float x1, float y1, float x2, float y2, float r, float g, float b, float a)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphplanar_t");
	
	if (unlikely(context == NULL))
		LOG_ERROR_AND_RETURN(-2, "null CGContextRef");
	
	CGContextBeginPath(context);
	CGContextSetLineWidth(context, 1.);
	CGContextSetRGBStrokeColor(context, r, g, b, a);
	
	CGContextMoveToPoint(context, x1, y1);
	CGContextAddLineToPoint(context, x2, y2);
	
	CGContextClosePath(context);
	CGContextDrawPath(context, kCGPathStroke);
	
	return 0;
}





@implementation PlanarGraphView

@synthesize graphController = mGraphController;
@synthesize isdirty = mIsDirty;
@synthesize tracking = mTracking;
@dynamic dspchain;
@dynamic dsptrim;
@synthesize trimSize = mTrimSize;

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
		mIsDirty = FALSE;
		mDevice = NULL;
		mTracking = TRUE;
		
		[[self window] setAcceptsMouseMovedEvents:TRUE];
		
		mTrackingArea = [[NSTrackingArea alloc] initWithRect:NSMakeRect(0., 0., frame.size.width, frame.size.height)
																								 options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow
																									 owner:self
																								userInfo:nil];
		[self addTrackingArea:mTrackingArea];
		
		[mResetBtn setAlphaValue:MAX_ALPHA];
		
		mAlpha = 1.;
		mFade = 0;
		mInside = TRUE;
	}
	
	return self;
}

/**
 *
 *
 */
- (void)dealloc
{
	if (mGraph != NULL) {
		graphplanar_release(mGraph);
		mGraph = NULL;
	}
	
	[super dealloc];
}





#pragma mark -
#pragma mark UI Callbacks

/**
 *
 *
 */
- (IBAction)doActionReset:(id)sender
{
	graphplanar_reset(mGraph);
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
	
	/*
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
	*/
	
	// draw the tracking lines
	if (mTracking == TRUE)
	{
		if (mouseInside == TRUE) {
			graph_value_t gvalue;
			size_t text_l = 0;
			char text[100];
			
			graph_get_value(graphplanar_graph(mGraph), (uint32_t)point.x, (uint32_t)point.y, &gvalue);
			
			// left and right sides "xx dBm"
			{
				text_l = (size_t)snprintf(text, 100, "%d dBm", (int)gvalue.yaxis);
				
				CGContextSelectFont(context, "Helvetica", 9., kCGEncodingMacRoman);
				CGContextSetCharacterSpacing(context, 2);
				CGContextSetTextDrawingMode(context, kCGTextFillStroke);
				CGContextSetLineWidth(context, 1.);
				CGContextSetRGBStrokeColor(context, 1., 0., 0., MAX_ALPHA);
				
				CGPoint textpos1 = CGContextGetTextPosition(context);
				CGContextSetTextDrawingMode(context, kCGTextInvisible);
				CGContextShowText(context, text, text_l);
				CGPoint textpos2 = CGContextGetTextPosition(context);
				int textw = (int)(textpos2.x - textpos1.x);
				
				CGContextSetTextDrawingMode(context, kCGTextFillStroke);
				
				// left
				CGContextBeginPath(context);
				CGContextMoveToPoint(context, 1., point.y);
				CGContextAddLineToPoint(context, LINE_LEN, point.y);
				CGContextClosePath(context);
				CGContextDrawPath(context, kCGPathStroke);
				CGContextShowTextAtPoint(context, 15., point.y-3., text, text_l);
				
				// right
				CGContextBeginPath(context);
				CGContextMoveToPoint(context, frame.size.width-1., point.y);
				CGContextAddLineToPoint(context, frame.size.width-LINE_LEN, point.y);
				CGContextClosePath(context);
				CGContextDrawPath(context, kCGPathStroke);
				CGContextDrawPath(context, kCGPathStroke);
				CGContextShowTextAtPoint(context, frame.size.width-15.-textw, point.y-3., text, text_l);
			}
			
			// top and bottom sides "xx MHz"
			{
				text_l = (size_t)snprintf(text, 100, "%lf MHz", gvalue.xaxis/1000000.);
				
				CGContextSetTextDrawingMode(context, kCGTextFillStroke);
				
				CGPoint textpos1 = CGContextGetTextPosition(context);
				CGContextSetTextDrawingMode(context, kCGTextInvisible);
				CGContextShowText(context, text, text_l);
				CGPoint textpos2 = CGContextGetTextPosition(context);
				int textw = (int)(textpos2.x - textpos1.x);
				
				CGContextSetTextDrawingMode(context, kCGTextFillStroke);
				
				// top
				CGContextBeginPath(context);
				CGContextMoveToPoint(context, point.x, frame.size.height-1.);
				CGContextAddLineToPoint(context, point.x, frame.size.height-LINE_LEN);
				CGContextClosePath(context);
				CGContextDrawPath(context, kCGPathStroke);
				CGContextShowTextAtPoint(context, point.x-(textw/2.), frame.size.height-LINE_LEN-4-5, text, text_l);
				
				// bottom
				CGContextBeginPath(context);
				CGContextMoveToPoint(context, point.x, 1.);
				CGContextAddLineToPoint(context, point.x, LINE_LEN);
				CGContextClosePath(context);
				CGContextDrawPath(context, kCGPathStroke);
				CGContextShowTextAtPoint(context, point.x-(textw/2.), LINE_LEN+4., text, text_l);
			}
		}
	}
	
	// fade the various controls
	if (mFade != 0) {
		struct timeval tv;
		double current;
		
		gettimeofday(&tv, NULL);
		
		current = ((double)tv.tv_sec*1000.) + ((double)tv.tv_usec/1000.);
		
		if (mInside) {
			if (mAlpha < MAX_ALPHA)
				mAlpha = MIN(MAX_ALPHA,mAlpha+((current - mFade) / 1000.));
			else
				mFade = 0;
		}
		else {
			if (mAlpha > 0.)
				mAlpha = MAX(0.,mAlpha-((current - mFade) / 1000.));
			else
				mFade = 0;
		}
		
		[mResetBtn setAlphaValue:mAlpha];
		[mResetBtn setNeedsDisplay:TRUE];
//	[mResetBtn drawRect:dirtyRect];
		
		if (mFade != 0)
			[self performSelector:@selector(redraw) withObject:nil afterDelay:0.1];
	}
	
	[super drawRect:dirtyRect];
}





#if 0

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
	
	if (NSAlternateKeyMask & flags)
		mSelect = TRUE;
	else
		mSelect = FALSE;
	
	if (unlikely(0 != (error = graph_get_value(graphplanar_graph(mGraph), (uint32_t)local_point.x, (uint32_t)local_point.y, &value))))
		LOG_ERROR_AND_RETURN(, "failed to graph_get_value, %d", error);
	
	mDragged = FALSE;
	mFrequencyBeg = (NSUInteger)value.xaxis;
	mFrequencyEnd = (NSUInteger)value.xaxis;
	mSelectBeg = local_point.x;
	mSelectEnd = local_point.x;
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
	
	if (mDragged) {
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
	else {
		mSelect = FALSE;
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
	
	if (unlikely(0 != (error = graph_get_value(graphplanar_graph(mGraph), (uint32_t)local_point.x, (uint32_t)local_point.y, &value))))
		LOG_ERROR_AND_RETURN(, "failed to graph_get_value(), %d", error);
	
	mDragged = TRUE;
	mFrequencyEnd = (NSUInteger)value.xaxis;
	mSelectEnd = local_point.x;
	
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

/**
 * keycode 123 = left arrow key
 * keycode 124 = right arrow key
 *
 */
/*
- (void)keyDown:(NSEvent *)theEvent
{
	uint16_t keycode = [theEvent keyCode];
	
	// left arrow
	if (123 == keycode) {
		if (mDspTrim->offset >= 128)
			mDspTrim->offset -= 128;
		else if (mDspTrim->offset > 0)
			mDspTrim->offset = 0;
		[self reset];
	}
	
	// right arrow
	else if (124 == keycode) {
		if (mDspTrim->offset <= (mTrimSize - mDspTrim->width - 128))
			mDspTrim->offset += 128;
		else if (mDspTrim->offset < (mTrimSize - mDspTrim->width))
			mDspTrim->offset = (uint32_t)(mTrimSize - mDspTrim->width);
		[self reset];
	}
	
	// not interesting
	else
		[[self nextResponder] keyDown:theEvent];
}
*/

/**
 *
 *
 */
/*
- (BOOL)acceptsFirstResponder
{
	return TRUE;
}
*/

#endif




#pragma mark -
#pragma mark Accessors

/**
 *
 *
 */
- (void)setDevice:(device_t*)device
{
	mDevice = device;
}

/**
 *
 *
 */
- (void)setGraph:(graphplanar_t *)graph
{
	if (mGraph != NULL)
		graphplanar_release(mGraph);
	
	mGraph = graphplanar_retain(graph);
	
	graph_set_redraw_fp(graphplanar_graph(graph), (graph_redraw_fp_func)__redraw, self);
	graph_set_draw_path_fp(graphplanar_graph(graph), (graph_draw_path_fp_func)__draw_path);
	graph_set_draw_line_fp(graphplanar_graph(graph), (graph_draw_line_fp_func)__draw_line);
}

/**
 *
 *
 */
- (void)setDspchain:(dspchain_t *)dspchain
{
	if (mDspChain == dspchain)
		return;
	
	if (mDspChain != NULL)
		dspchain_release(mDspChain);
	
	mDspChain = dspchain_retain(dspchain);
}

/**
 *
 *
 */
- (dspchain_t*)dspchain
{
	return mDspChain;
}

/**
 *
 *
 */
- (void)setDsptrim:(dsptrim_t *)dsptrim
{
	if (mDspTrim == dsptrim)
		return;
	
	if (mDspTrim != NULL)
		dsptrim_release(mDspTrim);
	
	mDspTrim = dsptrim_retain(dsptrim);
}

/**
 *
 *
 */
- (dsptrim_t*)dsptrim
{
	return mDspTrim;
}

/**
 *
 *
 */
- (void)reset
{
	graphplanar_reset(mGraph);
}

/**
 *
 *
 */
- (void)redraw
{
	[self setNeedsDisplay];
}

@end
