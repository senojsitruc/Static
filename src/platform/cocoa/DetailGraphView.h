//
//  DetailGraphView.h
//  Static
//
//  Created by Curtis Jones on 2010.11.27.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "PlanarGraphView.h"
#import "../../output/audio/coreaudio/coreaudio.h"

@interface DetailGraphView : PlanarGraphView
{
	BOOL mDragged;                       // did we drag while the mouse was down?
	BOOL mSelect;                        // are we selecting a range?
	BOOL mInsideSelection;               // were we inside the selected area on mouse-down?
	NSPoint mDownPt;                     // mouse-down point
	NSPoint mPrevPt;                     // previous location of mouse during drag
	CGFloat mSelectBeg;                  // x-axis select mouse-down point
	CGFloat mSelectEnd;                  // x-axis select mouse-up point (or current location)
	
	NSMenu *mSelectMenu;                 // contextual menu for selections
	
	coreaudio_t *coreaudio;              // playback
}

@end
