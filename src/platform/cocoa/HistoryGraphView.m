//
//  HistoryGraphView.m
//  Static
//
//  Created by Curtis Jones on 2010.01.08.
//  Copyright 2010 Curtis Jones. All rights reserved.
//

#import "HistoryGraphView.h"
#import "../../misc/logger.h"
#import <errno.h>
#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <sys/time.h>

#define MAX_ALPHA 0.8
#define LINE_LEN 11.





#pragma mark -
#pragma mark draw callbacks

/**
 *
 *
 */
static int
__redraw (graph_t *graph, HistoryGraphView *hgv)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graph_t");
	
	if (unlikely(hgv == NULL))
		LOG_ERROR_AND_RETURN(-2, "null HistoryGraphView");
	
	if (!hgv.isdirty)
		[hgv performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:FALSE];
	
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

/**
 *
 *
 */
static int
__draw_hist (graphhistory_t *graph, CGContextRef context, void *data, uint32_t width, uint32_t height)
{
	if (unlikely(graph == NULL))
		LOG_ERROR_AND_RETURN(-1, "null graphhistory_t");
	
	if (unlikely(context == NULL))
		LOG_ERROR_AND_RETURN(-2, "null CGContextRef");
	
	CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, data, width*height*4, NULL);
	CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	CGImageRef image = CGImageCreate(width, height, 8, 32, width*4, colorspace, kCGBitmapByteOrder32Little | kCGImageAlphaLast, provider, NULL, FALSE, kCGRenderingIntentPerceptual /*kCGRenderingIntentDefault*/ );
	
	CGContextDrawImage(context, NSMakeRect(0., 0., width, height), image);
	
	CGImageRelease(image);
	CGColorSpaceRelease(colorspace);
	CGDataProviderRelease(provider);
	
	return 0;
}





@implementation HistoryGraphView

@synthesize isdirty = mIsDirty;
@dynamic dspchain;
@dynamic dsptrim;
@synthesize trimSize = mTrimSize;

/**
 *
 *
 */
- (id)initWithFrame:(NSRect)frame
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	
	self = [super initWithFrame:frame];
	
	if (self) {
		mIsDirty = FALSE;
		
		[[self window] setAcceptsMouseMovedEvents:TRUE];
		
		mTrackingArea = [[NSTrackingArea alloc] initWithRect:NSMakeRect(0., 0., frame.size.width, frame.size.height)
																								 options:NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved|NSTrackingActiveInKeyWindow
																									 owner:self
																								userInfo:nil];
		[self addTrackingArea:mTrackingArea];
		
		[mResetBtn setAlphaValue:0.];
		
		mAlpha = MAX_ALPHA;
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
		graphhistory_release(mGraph);
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
	graphhistory_reset(mGraph);
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
	
//mGraph->graph.axis_y.length = [self frame].size.height;
	graph_draw(graphhistory_graph(mGraph), context);
	
	mIsDirty = FALSE;
	
	// draw the tracking lines
	{
		NSPoint point = [self convertPoint:[[self window] mouseLocationOutsideOfEventStream] fromView:nil];
		NSRect frame = [self frame];
		
		if (point.x > 0. && point.y > 0. && point.x < frame.size.width && point.y < frame.size.height) {
			CGContextSetLineWidth(context, 1.);
			CGContextSetRGBStrokeColor(context, 1., 0., 0., 0.8);
			
			// left
			CGContextBeginPath(context);
			CGContextMoveToPoint(context, 1., point.y);
			CGContextAddLineToPoint(context, LINE_LEN, point.y);
			CGContextClosePath(context);
			CGContextDrawPath(context, kCGPathStroke);
			
			// right
			CGContextBeginPath(context);
			CGContextMoveToPoint(context, frame.size.width-1., point.y);
			CGContextAddLineToPoint(context, frame.size.width-LINE_LEN, point.y);
			CGContextClosePath(context);
			CGContextDrawPath(context, kCGPathStroke);
			
			// top
			CGContextBeginPath(context);
			CGContextMoveToPoint(context, point.x, frame.size.height-1.);
			CGContextAddLineToPoint(context, point.x, frame.size.height-LINE_LEN);
			CGContextClosePath(context);
			CGContextDrawPath(context, kCGPathStroke);
			
			// bottom
			CGContextBeginPath(context);
			CGContextMoveToPoint(context, point.x, 1.);
			CGContextAddLineToPoint(context, point.x, LINE_LEN);
			CGContextClosePath(context);
			CGContextDrawPath(context, kCGPathStroke);
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





#pragma mark -
#pragma mark NSResonder

/**
 *
 *
 */
- (void)mouseUp:(NSEvent *)theEvent
{
	NSLog(@"clickCount = %ld", theEvent.clickCount);
	
	if (theEvent.clickCount == 2) {
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		
		NSLog(@"x=%f, y=%f", local_point.x, local_point.y);
	}
}

/**
 *
 *
 */
- (void)mouseDown:(NSEvent *)theEvent
{
	NSLog(@"clickCount = %ld", theEvent.clickCount);
	
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





#pragma mark -
#pragma mark accessors

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
- (void)setGraph:(graphhistory_t *)graph
{
	if (mGraph != NULL)
		graphhistory_release(mGraph);
	
	mGraph = graphhistory_retain(graph);
	
	graph_set_redraw_fp(graphhistory_graph(graph), (graph_redraw_fp_func)__redraw, self);
	graph_set_draw_hist_fp(graphhistory_graph(graph), (graph_draw_hist_fp_func)__draw_hist);
	graph_set_draw_path_fp(graphhistory_graph(graph), (graph_draw_path_fp_func)__draw_hist);
	graph_set_draw_line_fp(graphhistory_graph(graph), (graph_draw_line_fp_func)__draw_line);
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
	graphhistory_reset(mGraph);
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
