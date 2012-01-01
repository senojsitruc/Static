//
//  GraphController.m
//  Static
//
//  Created by Curtis Jones on 2009.12.29.
//  Copyright 2009 Curtis Jones. All rights reserved.
//

#import "GraphController.h"
#import "../../misc/logger.h"
#import "../../protocol/ascp/ascp.h"
#import "DetailGraphView.h"
#import "HistoryGraphView.h"
#import "OverviewGraphView.h"
#import "PlanarGraphView.h"
#import <errno.h>
#import <stdio.h>
#import <stdlib.h>
#import <string.h>

@implementation GraphController

@synthesize window = mWindow;
@synthesize planarview = mPlanarGraphView;
@synthesize planarview2 = mPlanarGraphView2;
@synthesize historyview = mHistoryGraphView;





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
- (id)init
{
	self = [super init];
	
	if (self) {
		// ...
	}
	
	return self;
}

/**
 *
 *
 */
- (void)awakeFromNib
{
	[mWindow setLevel:NSNormalWindowLevel];
}

/**
 *
 *
 */
- (void)dealloc
{
	graphhistory_release(mGraphHistory);
	graphplanar_release(mGraphPlanar);
	graphplanar_release(mGraphPlanar2);
	
	mGraphHistory = NULL;
	mGraphPlanar = NULL;
	mGraphPlanar2 = NULL;
	
	[super dealloc];
}





#pragma mark -
#pragma mark other

/**
 * Called when the selection box in the overview graph is dragged.
 *
 * Always adjust in multiples of sixteen because each "point" is represented by an I/Q value pair,
 * each of which is an 8-byte double (ie, sizeof(double) * 2 = 16)
 *
 */
- (void)overviewGraphChangedSelection:(OverviewGraphView *)overviewGraphView
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	
	NSInteger frequencyRange = mFrequencyHi - mFrequencyLo;
	NSInteger graphWidth = (NSInteger)[mPlanarGraphView2 frame].size.width;
	NSInteger dataWidth = ASCP_IQ_COUNT * 2;
	NSInteger offset = 0;
	
	if (overviewGraphView.selectBeg > 0.) {
		offset = (NSInteger)((CGFloat)dataWidth * (overviewGraphView.selectBeg / (CGFloat)graphWidth)) * 16;
		offset -= offset % 16;
		
		mGraphPlanar->graph.axis_x.unit_low  = mFrequencyLo + (frequencyRange * (overviewGraphView.selectBeg / graphWidth));
		mGraphPlanar->graph.axis_x.unit_high = mFrequencyLo + (frequencyRange * (overviewGraphView.selectBeg / graphWidth)) + (frequencyRange / 8);
		
		mGraphHistory->graph.axis_x.unit_low  = mFrequencyLo + (frequencyRange * (overviewGraphView.selectBeg / graphWidth));
		mGraphHistory->graph.axis_x.unit_high = mFrequencyLo + (frequencyRange * (overviewGraphView.selectBeg / graphWidth)) + (frequencyRange / 8);
	}
	else {
		mGraphPlanar->graph.axis_x.unit_low  = mFrequencyLo;
		mGraphPlanar->graph.axis_x.unit_high = mFrequencyLo + (frequencyRange / 4);
		
		mGraphHistory->graph.axis_x.unit_low  = mFrequencyLo;
		mGraphHistory->graph.axis_x.unit_high = mFrequencyLo + (frequencyRange / 4);
	}
	
	if (mPlanarGraphView.dsptrim != NULL) {
		mPlanarGraphView.dsptrim->offset = (uint32_t)offset;
		[mPlanarGraphView reset];
	}
	
	if (mHistoryGraphView.dsptrim != NULL) {
		mHistoryGraphView.dsptrim->offset = (uint32_t)offset;
		[mHistoryGraphView reset];
	}
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
- (void)redraw
{
	[mHistoryGraphView setNeedsDisplay:TRUE];
	[mPlanarGraphView setNeedsDisplay:TRUE];
	[mPlanarGraphView2 setNeedsDisplay:TRUE];
}

/**
 *
 *
 */
- (void)reset
{
	[mHistoryGraphView reset];
	[mPlanarGraphView reset];
	[mPlanarGraphView2 reset];
}

/**
 *
 *
 */
- (void)setLabel:(NSString *)label
{
	[mLabelTxt performSelectorOnMainThread:@selector(setStringValue:) withObject:label waitUntilDone:FALSE];
}

/**
 *
 *
 */
- (void)setSampleCount:(NSInteger)sampleCount
{
	//[mSampleCountTxt performSelectorOnMainThread:@selector(setObjectValue:) withObject:[NSNumber numberWithInteger:sampleCount] waitUntilDone:FALSE];
}

/**
 *
 *
 */
- (graphplanar_t*)graphplanar
{
	return mGraphPlanar;
}

/**
 *
 *
 */
- (void)setGraphplanar:(graphplanar_t*)graphplanar
{
	if (mGraphPlanar != NULL)
		graphplanar_release(mGraphPlanar);
	
	mGraphPlanar = graphplanar_retain(graphplanar);
	[mPlanarGraphView setGraph:mGraphPlanar];
}

/**
 *
 *
 */
- (graphplanar_t*)graphplanar2
{
	return mGraphPlanar2;
}

/**
 *
 *
 */
- (void)setGraphplanar2:(graphplanar_t*)graphplanar
{
	if (mGraphPlanar2 != NULL)
		graphplanar_release(mGraphPlanar2);
	
	mGraphPlanar2 = graphplanar_retain(graphplanar);
	[mPlanarGraphView2 setGraph:mGraphPlanar2];
}

/**
 *
 *
 */
- (graphhistory_t*)graphhistory
{
	return mGraphHistory;
}

/**
 *
 *
 */
- (void)setGraphhistory:(graphhistory_t*)graphhistory
{
	if (mGraphHistory != NULL)
		graphhistory_release(mGraphHistory);
	
	mGraphHistory = graphhistory_retain(graphhistory);
	[mHistoryGraphView setGraph:mGraphHistory];
}

/**
 *
 *
 */
- (void)setFrequencyHigh:(NSInteger)frequencyHi low:(NSInteger)frequencyLo;
{
	mFrequencyHi = frequencyHi;
	mFrequencyLo = frequencyLo;
}

@end
