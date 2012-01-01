//
//  PlanarGraphView.h
//  Static
//
//  Created by Curtis Jones on 2009.12.29.
//  Copyright 2009 Curtis Jones. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "../../device/device.h"
#import "../../dsp/dspchain.h"
#import "../../dsp/other/trim/trim.h"
#import "../../output/graph/graph.h"
#import "../../output/graph/planar/planar.h"

@class GraphController;

@interface PlanarGraphView : NSControl
{
	GraphController *mGraphController;   // graph controller
	
	device_t *mDevice;                   // 
	dspchain_t *mDspChain;               // dsp chain
	dsptrim_t *mDspTrim;                 // dsp trim
	graphplanar_t *mGraph;               // 
	NSUInteger mTrimSize;                // size of data being fed to dsptrim
	
	BOOL mIsDirty;                       // do we need to re-draw?
//BOOL mDragged;                       // did we drag while the mouse was down?
//BOOL mSelect;                        // are we selecting a range?
	BOOL mTracking;                      // enable frequency / amplitude tracking
	NSUInteger mFrequencyBeg;            // mouse-down location
	NSUInteger mFrequencyEnd;            // current mouse location or mouse-up location
//CGFloat mSelectBeg;                  // x-axis select mouse-down point
//CGFloat mSelectEnd;                  // x-axis select mouse-up point (or current location)
	
	uint64_t mFade;                      // fade timer
	CGFloat mAlpha;                      // alpha value
	NSTrackingArea *mTrackingArea;       // tracking area
	BOOL mInside;                        // is the mouse inside the view?
	
	IBOutlet NSButton *mResetBtn;        // reset button in top-right corner
}

@property (readwrite, retain) GraphController *graphController;

@property (readonly) BOOL isdirty;
@property (readwrite, assign) BOOL tracking;
@property (readwrite, assign) dspchain_t *dspchain;
@property (readwrite, assign) dsptrim_t *dsptrim;
@property (readwrite, assign) NSUInteger trimSize;

- (void)setDevice:(device_t*)device;
- (void)setGraph:(graphplanar_t*)graph;
- (void)reset;
- (void)redraw;

- (IBAction)doActionReset:(id)sender;

@end
