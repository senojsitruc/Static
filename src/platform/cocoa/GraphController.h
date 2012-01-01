//
//  GraphController.h
//  Static
//
//  Created by Curtis Jones on 2009.12.29.
//  Copyright 2009 Curtis Jones. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "../../output/graph/graph.h"
#import "../../output/graph/history/history.h"
#import "../../output/graph/planar/planar.h"

@class DetailGraphView;
@class HistoryGraphView;
@class OverviewGraphView;
@class PlanarGraphView;

@interface GraphController : NSObject
{
	IBOutlet DetailGraphView *mPlanarGraphView;    // planar
	IBOutlet OverviewGraphView *mPlanarGraphView2; // overview
	IBOutlet HistoryGraphView *mHistoryGraphView;  // history
	IBOutlet NSTextField *mLabelTxt;
	IBOutlet NSPanel *mWindow;
	
	graphplanar_t *mGraphPlanar;
	graphplanar_t *mGraphPlanar2;
	graphhistory_t *mGraphHistory;
	
	NSInteger mFrequencyHi;
	NSInteger mFrequencyLo;
}

@property (readonly) NSPanel *window;
@property (readwrite) graphplanar_t *graphplanar;
@property (readwrite) graphplanar_t *graphplanar2;
@property (readwrite) graphhistory_t *graphhistory;
@property (readonly) DetailGraphView *planarview;
@property (readonly) OverviewGraphView *planarview2;
@property (readonly) HistoryGraphView *historyview;

- (void)reset;
- (void)setLabel:(NSString *)label;
- (void)setSampleCount:(NSInteger)sampleCount;
- (void)redraw;
- (void)setFrequencyHigh:(NSInteger)frequencyHi low:(NSInteger)frequencyLo;
- (void)overviewGraphChangedSelection:(OverviewGraphView *)overviewGraphView;

@end
