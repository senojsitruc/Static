//
//  DeviceController.h
//  Static
//
//  Created by Curtis Jones on 2010.01.01.
//  Copyright 2010 Curtis Jones. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol DeviceProtocol
- (void)deviceFrequency:(NSInteger)frequency context:(void *)context;
- (void)deviceGain:(NSInteger)gain context:(void *)context;
- (void)deviceSpan:(NSInteger)span context:(void *)context;
- (void)deviceUnit:(NSInteger)unit context:(void *)context;
@end

@interface DeviceController : NSObject
{
	IBOutlet NSPanel *mWindow;
	IBOutlet NSButton *mStartBtn;							// start stop button
	IBOutlet NSSlider *mSpanSlider;						// frequency span
	IBOutlet NSSlider *mGainSlider;						// rf gain in db
	IBOutlet NSTextField *mSpanTxt;						// frequency span
	IBOutlet NSTextField *mGainTxt;						// rf gain in db
	IBOutlet NSTextField *mFreqTxt;						// frequency
	IBOutlet NSSegmentedControl *mUnitCtrl;		// hz, khz, mhz
	IBOutlet NSTextField *mDeviceTxt;					// device name
	IBOutlet NSTextField *mSerialTxt;					// serial number
	IBOutlet NSTextField *mInterfaceTxt;			// interface version
	IBOutlet NSTextField *mHardwareTxt;				// hardware version
	IBOutlet NSTextField *mFirmwareTxt;				// firmware version
	IBOutlet NSTextField *mDriverTxt;					// driver name
	
	id <DeviceProtocol> delegate;
	void *context;
	
	BOOL mStarted;														// 
	
	NSInteger mFrequencyLo;                   // left-side frequency (low)
	NSInteger mFrequencyHi;                   // right-side frequency (high)
}

@property (readonly) NSPanel *window;

@property (readwrite, assign) id <DeviceProtocol> delegate;
@property (readwrite) void *context;

@property (readwrite) NSInteger span;
@property (readwrite) NSInteger gain;
@property (readwrite) NSInteger frequency;
@property (readwrite, assign) NSInteger frequencyLo;
@property (readwrite, assign) NSInteger frequencyHi;
@property (readwrite, assign) NSString *device;
@property (readwrite, assign) NSString *serial;
@property (readwrite, assign) NSString *driver;
@property (readwrite, assign) NSString *hardware;
@property (readwrite, assign) NSString *interface;
@property (readwrite, assign) NSString *firmware;

- (void)setFrequency:(NSInteger)frequency;
- (NSInteger)frequency;

- (void)setSpan:(NSInteger)span;
- (NSInteger)span;

- (void)setGain:(NSInteger)gain;
- (NSInteger)gain;

- (IBAction)doActionStart:(id)sender;

@end
