//
//  AppController.h
//  Static
//
//  Created by Curtis Jones on 2009.12.20.
//  Copyright 2009 Curtis Jones. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "device/device.h"
#import "misc/event/eventhandler.h"
#import "platform/cocoa/GraphController.h"
#import "platform/cocoa/DeviceController.h"
#import "platform/cocoa/DevicesController.h"
#import "platform/cocoa/DetailGraphView.h"
#import "platform/cocoa/OverviewGraphView.h"
#import "output/graph/planar/planar.h"
#import "output/graph/history/history.h"
#import "output/audio/coreaudio/coreaudio.h"

@interface AppController : NSObject <DeviceProtocol>
{
	IBOutlet GraphController *mGraphController;
	IBOutlet DeviceController *mDeviceController;
	IBOutlet DevicesController *mDevicesController;
	
	device_t *mDevice;							// device
	eventhandler_t *mHandler;				// event handler
	
	graphplanar_t *mPlanar;					// 
	graphplanar_t *mPlanar2;        // overview
	graphhistory_t *mHistory;				// 
	
	BOOL mStop;											// 
}

@property (readonly) DeviceController* deviceController;
@property (readonly) GraphController* graphController;
@property (readonly) device_t* device;
@property (readonly) graphplanar_t* planar;
@property (readonly) graphplanar_t* planar2;
@property (readonly) graphhistory_t* history;

- (void)stop;
- (void)setup;

- (IBAction)doActionInfo:(id)sender;
- (IBAction)doActionInfoClose:(id)sender;

@end
