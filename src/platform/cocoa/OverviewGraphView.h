//
//  OverviewGraphView.h
//  Static
//
//  Created by Curtis Jones on 2010.11.27.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "PlanarGraphView.h"

@interface OverviewGraphView : PlanarGraphView
{
	BOOL mDragged;                       // did we drag while the mouse was down?
	CGFloat mSelectBeg;                  // x-axis select mouse-down point
	CGFloat mSelectEnd;                  // x-axis select mouse-up point (or current location)
	NSPoint mDownPt;                     // 
	NSPoint mLastPt;                     // 
}

@property (readwrite, assign) CGFloat selectBeg;
@property (readwrite, assign) CGFloat selectEnd;

@end
