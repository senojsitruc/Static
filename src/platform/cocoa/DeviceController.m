//
//  DeviceController.m
//  Static
//
//  Created by Curtis Jones on 2010.01.01.
//  Copyright 2010 Curtis Jones. All rights reserved.
//

#import "DeviceController.h"

@implementation DeviceController

@synthesize window = mWindow;
@synthesize delegate;
@synthesize context;
@synthesize frequencyLo = mFrequencyLo;
@synthesize frequencyHi = mFrequencyHi;

/**
 *
 *
 */
- (void)awakeFromNib
{
	mStarted = FALSE;
	
	[self setSpan:190];
	[self setGain:0];
	[self setFrequency:5000000];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(doActionFreq:) name:@"NSControlTextDidEndEditingNotification" object:mFreqTxt];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(doHandleAD6620Started:) name:@"AD6620Started" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(doHandleAD6620Stopped:) name:@"AD6620Stopped" object:nil];
	
	[mSpanSlider setTarget:self];
	[mSpanSlider setAction:@selector(doActionSpan:)];
	[mGainSlider setTarget:self];
	[mGainSlider setAction:@selector(doActionGain:)];
	[mUnitCtrl setTarget:self];
	[mUnitCtrl setAction:@selector(doActionUnit:)];
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
- (void)setFrequency:(NSInteger)frequency
{
	mFrequencyLo = frequency - (190000 / 2);
	mFrequencyHi = frequency + (190000 / 2);
	
	[mFreqTxt performSelectorOnMainThread:@selector(setObjectValue:) withObject:[NSNumber numberWithInteger:frequency] waitUntilDone:TRUE];
}

/**
 *
 *
 */
- (NSInteger)frequency
{
	return [mFreqTxt integerValue];
}

/**
 *
 *
 */
- (void)setSpan:(NSInteger)span
{
	NSNumber *spanNumber = nil;
	
	if (span == 5)
		spanNumber = [NSNumber numberWithInteger:7];
	
	else if (span == 10)
		spanNumber = [NSNumber numberWithInteger:6];
	
	else if (span == 25)
		spanNumber = [NSNumber numberWithInteger:5];
	
	else if (span == 50)
		spanNumber = [NSNumber numberWithInteger:4];
	
	else if (span == 100)
		spanNumber = [NSNumber numberWithInteger:3];
	
	else if (span == 150)
		spanNumber = [NSNumber numberWithInteger:2];
	
	else if (span == 190)
		spanNumber = [NSNumber numberWithInteger:1];
	
	if (spanNumber != nil) {
		[mSpanSlider performSelectorOnMainThread:@selector(setObjectValue:) withObject:spanNumber waitUntilDone:TRUE];
		[self performSelectorOnMainThread:@selector(doActionSpan:) withObject:nil waitUntilDone:TRUE];
	}
}

/**
 *
 *
 */
- (NSInteger)span
{
	NSInteger span = [mSpanSlider integerValue];
	
	if (span == 7)
		span = 5;
	
	else if (span == 6)
		span = 10;
	
	else if (span == 5)
		span = 25;
	
	else if (span == 4)
		span = 50;
	
	else if (span == 3)
		span = 100;
	
	else if (span == 2)
		span = 150;
	
	else if (span == 1)
		span = 190;
	
	else
		span = 0;
	
	return span;
}

/**
 *
 *
 */
- (void)setGain:(NSInteger)gain
{
	NSNumber *gainNumber = nil;
	
	if (gain == -30)
		gainNumber = [NSNumber numberWithInteger:4];
	
	else if (gain == -20)
		gainNumber = [NSNumber numberWithInteger:3];
	
	else if (gain == -10)
		gainNumber = [NSNumber numberWithInteger:2];
	
	else if (gain == 0)
		gainNumber = [NSNumber numberWithInteger:1];
	
	if (gainNumber != nil) {
		[mGainSlider performSelectorOnMainThread:@selector(setObjectValue:) withObject:gainNumber waitUntilDone:TRUE];
		[self performSelectorOnMainThread:@selector(doActionGain:) withObject:nil waitUntilDone:TRUE];
	}
}

/**
 *
 *
 */
- (NSInteger)gain
{
	NSInteger gain = [mGainSlider integerValue];
	
	if (gain == 4)
		gain = -30;
	
	else if (gain == 3)
		gain = -20;
	
	else if (gain == 2)
		gain = -10;
	
	else if (gain == 1)
		gain = 0;
	
	else
		gain = 0;
	
	return gain;
}

- (void)setDevice:(NSString *)device
{
	[mDeviceTxt setStringValue:device];
}

- (NSString *)device
{
	return [mDeviceTxt stringValue];
}

- (void)setSerial:(NSString *)serial
{
	[mSerialTxt setStringValue:serial];
}

- (NSString *)serial
{
	return [mSerialTxt stringValue];
}

- (void)setDriver:(NSString *)driver
{
	[mDriverTxt setStringValue:driver];
}

- (NSString *)driver
{
	return [mDriverTxt stringValue];
}

- (void)setInterface:(NSString *)interface
{
	[mInterfaceTxt setStringValue:interface];
}

- (NSString *)interface
{
	return [mInterfaceTxt stringValue];
}

- (void)setHardware:(NSString *)hardware
{
	[mHardwareTxt setStringValue:hardware];
}

- (NSString *)hardware
{
	return [mHardwareTxt stringValue];
}

- (void)setFirmware:(NSString *)firmware
{
	[mFirmwareTxt setStringValue:firmware];
}

- (NSString *)firmware
{
	return [mFirmwareTxt stringValue];
}





#pragma mark -
#pragma mark action callbacks

/**
 *
 *
 */
- (void)doActionSpan:(id)sender
{
	[mSpanTxt setStringValue:[NSString stringWithFormat:@"%d KHz", [self span]]];
	[delegate deviceSpan:[self span] context:context];
}

/**
 *
 *
 */
- (void)doActionGain:(id)sender
{
	[mGainTxt setStringValue:[NSString stringWithFormat:@"%d dB", [self gain]]];
	[delegate deviceGain:[self gain] context:context];
}

/**
 *
 *
 */
- (void)doActionUnit:(id)sender
{
	[delegate deviceUnit:[mUnitCtrl selectedSegment] context:context];
}

/**
 *
 *
 */
- (void)doActionFreq:(NSNotification *)notification
{
	[delegate deviceFrequency:[self frequency] context:context];
}

/**
 *
 *
 */
- (IBAction)doActionStart:(id)sender
{
	if (mStarted) {
		[mStartBtn setEnabled:FALSE];
		[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Stop" object:self];
	}
	else {
		[mStartBtn setEnabled:FALSE];
		[mSpanSlider setEnabled:FALSE];
		[mGainSlider setEnabled:FALSE];
		[[NSNotificationCenter defaultCenter] postNotificationName:@"AD6620Start" object:self];
	}
}

- (void)doHandleAD6620Started:(NSNotification *)notification
{
	mStarted = TRUE;
	[mStartBtn setTitle:@"Stop"];
	[mStartBtn setEnabled:TRUE];
}

- (void)doHandleAD6620Stopped:(NSNotification *)notification
{
	mStarted = FALSE;
	[mStartBtn setTitle:@"Start"];
	[mStartBtn setEnabled:TRUE];
	[mSpanSlider setEnabled:TRUE];
	[mGainSlider setEnabled:TRUE];
}

@end
