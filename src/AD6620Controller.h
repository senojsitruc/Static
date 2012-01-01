//
//  AD6620Controller.h
//  Static
//
//  Created by Curtis Jones on 2009.12.28.
//  Copyright 2009 Curtis Jones. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AD6620Controller : NSObject
{
	IBOutlet NSSlider *mCIC2;					// Scic2
	IBOutlet NSSlider *mCIC5;					// Scic5
	IBOutlet NSSlider *mRCF;					// RCF rate
	IBOutlet NSSlider *mSpan;					// Span (in KHz)
	
	IBOutlet NSTextField *mCIC2Txt;		// 
	IBOutlet NSTextField *mCIC5Txt;		// 
	IBOutlet NSTextField *mRCFTxt;		// 
	IBOutlet NSTextField *mSpanTxt;		// 
	
	IBOutlet NSButton *mStartBtn;			// 
	IBOutlet NSButton *mApplyBtn;			// 
	
	BOOL mStarted;										// 
}

- (IBAction)doActionStart:(id)sender;
- (IBAction)doActionApply:(id)sender;

@end
