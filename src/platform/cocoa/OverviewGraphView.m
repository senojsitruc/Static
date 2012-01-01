//
//  OverviewGraphView.m
//  Static
//
//  Created by Curtis Jones on 2010.11.27.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "OverviewGraphView.h"
#import "../../misc/logger.h"
#import "GraphController.h"
#import <errno.h>
#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <sys/time.h>

#define MAX_ALPHA .8
#define LINE_LEN 11.

@implementation OverviewGraphView

@synthesize selectBeg = mSelectBeg;
@synthesize selectEnd = mSelectEnd;





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
	if (mSelectEnd != mSelectBeg) {
		CGFloat selectLo, selectHi;
		
		if (mSelectBeg < mSelectEnd) {
			selectLo = mSelectBeg;
			selectHi = mSelectEnd;
		}
		else {
			selectLo = mSelectEnd;
			selectHi = mSelectBeg;
		}
		
		// the box
		{
			CGContextBeginPath(context);
			CGContextSetLineWidth(context, 1.);
			CGContextSetRGBStrokeColor(context, 1., 1., 1., MAX_ALPHA);
			CGContextSetRGBFillColor(context, .8, .8, .8, 0.4);
			
			CGContextMoveToPoint(context, mSelectBeg+2, 2.);
			CGContextAddLineToPoint(context, mSelectBeg+2, frame.size.height-3);
			CGContextAddLineToPoint(context, mSelectEnd, frame.size.height-3);
			CGContextAddLineToPoint(context, mSelectEnd, 2.);
			CGContextClosePath(context);
			
			CGContextDrawPath(context, kCGPathFillStroke);
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
//int error;
//graph_value_t value;
	NSPoint event_location = [theEvent locationInWindow];
	NSPoint local_point = [self convertPoint:event_location fromView:nil];
//NSUInteger flags = [theEvent modifierFlags];
	
	if (local_point.x >= mSelectBeg && local_point.x <= mSelectEnd) {
		mDragged = TRUE;
		mDownPt = local_point;
		mLastPt = local_point;
	}
	
	/*
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
	*/
	
}

/**
 *
 *
 */
- (void)mouseUp:(NSEvent *)theEvent
{
	if (mDevice == NULL)
		return;
	
	if (mDragged) {
		/*
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		
		CGFloat delta_x = local_point.x - mDownPt.x;
		
		mSelectBeg += delta_x;
		mSelectEnd += delta_x;
		*/
		
		[mGraphController overviewGraphChangedSelection:self];
		
		mDragged = FALSE;
	}
	
	/*
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
	*/
	
	[self performSelectorOnMainThread:@selector(redraw) withObject:nil waitUntilDone:FALSE];
}

/**
 *
 *
 */
- (void)mouseDragged:(NSEvent *)theEvent
{
	/*
	int error;
	graph_value_t value;
	NSPoint event_location = [theEvent locationInWindow];
	NSPoint local_point = [self convertPoint:event_location fromView:nil];
	
	if (unlikely(0 != (error = graph_get_value(graphplanar_graph(mGraph), (uint32_t)local_point.x, (uint32_t)local_point.y, &value))))
		LOG_ERROR_AND_RETURN(, "failed to graph_get_value(), %d", error);
	
	mDragged = TRUE;
	mFrequencyEnd = (NSUInteger)value.xaxis;
	mSelectEnd = local_point.x;
	*/
	
	if (mDragged) {
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		
		CGFloat delta_x = local_point.x - mLastPt.x;
		
		if (mSelectBeg + delta_x < 0 || mSelectEnd + delta_x > [self frame].size.width)
			return;
		
		mLastPt = local_point;
		mSelectBeg += delta_x;
		mSelectEnd += delta_x;
		
//	[mGraphController overviewGraphChangedSelection:self];
		
		[self performSelectorOnMainThread:@selector(redraw) withObject:nil waitUntilDone:FALSE];
	}
}

#if 0

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

#endif

@end
