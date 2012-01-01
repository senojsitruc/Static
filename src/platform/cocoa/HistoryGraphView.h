//
//  HistoryGraphView.h
//  Static
//
//  Created by Curtis Jones on 2010.01.08.
//  Copyright 2010 Curtis Jones. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "../../device/device.h"
#import "../../dsp/dspchain.h"
#import "../../dsp/other/trim/trim.h"
#import "../../output/graph/graph.h"
#import "../../output/graph/history/history.h"

@interface HistoryGraphView : NSControl
{
	device_t *mDevice;                   // 
	dspchain_t *mDspChain;               // dsp chain
	dsptrim_t *mDspTrim;                 // dsp trim
	graphhistory_t *mGraph;						// 
	NSUInteger mTrimSize;                // size of data being fed to dsptrim
	
	BOOL mIsDirty;										// do we need to re-draw?
	
	IBOutlet NSButton *mResetBtn;			// 
	
	uint64_t mFade;										// fade timer
	CGFloat mAlpha;										// alpha value
	NSTrackingArea *mTrackingArea;		// tracking area
	BOOL mInside;											// is the mouse inside the view?
}

@property (readonly) BOOL isdirty;
@property (readwrite, assign) dspchain_t *dspchain;
@property (readwrite, assign) dsptrim_t *dsptrim;
@property (readwrite, assign) NSUInteger trimSize;

- (void)setDevice:(device_t*)device;
- (void)setGraph:(graphhistory_t*)graph;
- (void)reset;
- (void)redraw;

- (IBAction)doActionReset:(id)sender;

@end
