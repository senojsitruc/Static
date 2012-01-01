//
//  OutputAudioController.h
//  Static
//
//  Created by Curtis Jones on 2010.01.16.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface OutputAudioController : NSObject
{
	IBOutlet NSTextField *mDeviceTxt;
	IBOutlet NSTextField *mFrequencyTxt;
	IBOutlet NSTextField *mSpanTxt;
	IBOutlet NSButton *mApplyBtn;
	IBOutlet NSButton *mStartBtn;
	IBOutlet NSSegmentedControl *mFrequencyUnitCtrl;
	IBOutlet NSSegmentedControl *mSpanUnitCtrl;
}

@end
