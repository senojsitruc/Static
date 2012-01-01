//
//  AD6620Controller.m
//  Static
//
//  Created by Curtis Jones on 2009.12.28.
//  Copyright 2009 Curtis Jones. All rights reserved.
//

#import "AD6620Controller.h"

//These tables are used to calculate the decimation stage gains
// which are a function of the decimation rate of each stage
const int gCIC2Scale[17] = {
	0,
	0,0,2,2,	//1-16
	3,4,4,4,
	5,5,5,6,
	6,6,6,6
};
const int gCIC5Scale[33] = {
	0,
	0,  0, 3, 5,	//1-32
	7,  8,10,10,
	11,12,13,13,
	14,15,15,15,
	16,16,17,17,
	17,18,18,18,
	19,19,19,20,
	20,20,20,20
};

@interface AD6620Controller (PrivateMethods)
- (void)doActionSlider:(NSSlider *)slider;
@end

@implementation AD6620Controller

- (void)awakeFromNib
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	
	mStarted = FALSE;
	
	[mCIC2 setTarget:self];
	[mCIC2 setAction:@selector(doActionSlider:)];
	
	[mCIC5 setTarget:self];
	[mCIC5 setAction:@selector(doActionSlider:)];
	
	[mRCF setTarget:self];
	[mRCF setAction:@selector(doActionSlider:)];
	
	[mSpan setTarget:self];
	[mSpan setAction:@selector(doActionSlider:)];
	
	[self doActionSlider:mCIC2];
	[self doActionSlider:mCIC5];
	[self doActionSlider:mRCF];
	[self doActionSlider:mSpan];
	
	[mStartBtn setEnabled:TRUE];
	[mApplyBtn setEnabled:FALSE];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(doHandleAD6620Started:) name:@"AD6620Started" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(doHandleAD6620Stopped:) name:@"AD6620Stopped" object:nil];
}

- (void)doActionSlider:(NSSlider *)slider
{
	int val = [slider intValue];
	
	if (slider == mCIC2) {
		[mCIC2Txt setIntValue:val];
		
	}
	
	else if (slider == mCIC5) {
		[mCIC5Txt setIntValue:val];
		
	}
	
	else if (slider == mRCF) {
		[mRCFTxt setIntValue:val];
		
	}
	
	else if (slider == mSpan) {
		if (val == 1)
			val = 5;
		else if (val == 2)
			val = 10;
		else if (val == 3)
			val = 25;
		else if (val == 4)
			val = 50;
		else if (val == 5)
			val = 100;
		else if (val == 6)
			val = 150;
		else if (val == 7)
			val = 190;
		
		[mSpanTxt setIntValue:val];
	}
	
	[mStartBtn setEnabled:FALSE];
	[mApplyBtn setEnabled:TRUE];
}

- (void)doHandleAD6620Started:(NSNotification *)notification
{
	mStarted = TRUE;
	
	[mStartBtn setTitle:@"Stop"];
	[mStartBtn setEnabled:TRUE];
	
	[mCIC2 setEnabled:FALSE];
	[mCIC5 setEnabled:FALSE];
	[mRCF setEnabled:FALSE];
	[mSpan setEnabled:FALSE];
}

- (void)doHandleAD6620Stopped:(NSNotification *)notification
{
	mStarted = FALSE;
	
	[mStartBtn setTitle:@"Start"];
	[mStartBtn setEnabled:TRUE];
	
	[mCIC2 setEnabled:TRUE];
	[mCIC5 setEnabled:TRUE];
	[mRCF setEnabled:TRUE];
	[mSpan setEnabled:TRUE];
}

- (IBAction)doActionStart:(id)sender
{
	NSLog(@"sender = %@", sender);
	
	if (mStarted) {
		[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Stop" object:self];
	}
	else {
		[mStartBtn setEnabled:FALSE];
		[mApplyBtn setEnabled:FALSE];
		[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Start" object:self];
	}
}

- (IBAction)doActionApply:(id)sender
{
	NSLog(@"sender = %@", sender);
	
	int span = [mSpanTxt intValue];
	int cic2 = [mCIC2Txt intValue];
	int cic5 = [mCIC5Txt intValue];
	
	     if (span == 1) span = 5;
	else if (span == 2) span = 10;
	else if (span == 3) span = 25;
	else if (span == 4) span = 50;
	else if (span == 5) span = 100;
	else if (span == 6) span = 150;
	else if (span == 7) span = 190;
	
	NSDictionary *info = [NSDictionary dictionaryWithObjectsAndKeys:
												[NSNumber numberWithInt:gCIC2Scale[cic2]],
												@"scic2",
												[NSNumber numberWithInt:cic2-1],
												@"mcic2",
												[NSNumber numberWithInt:gCIC5Scale[cic5]],
												@"scic5",
												[NSNumber numberWithInt:cic5-1],
												@"mcic5",
												[NSNumber numberWithInt:[mRCFTxt intValue]],
												@"rcf",
												[NSNumber numberWithInt:span],
												@"span",
												nil
											];
	
	[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Params" object:self userInfo:info];
	
	[mStartBtn setEnabled:TRUE];
	[mApplyBtn setEnabled:FALSE];
}

@end
