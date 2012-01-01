//
//  DevicesController.h
//  Static
//
//  Created by Curtis Jones on 2010.01.02.
//  Copyright 2010 Curtis Jones. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface DevicesController : NSObject <NSTableViewDelegate, NSTableViewDataSource>
{
	IBOutlet NSWindow *mWindow;				// window
	IBOutlet NSTableView *mTable;			// device list
}

@end
