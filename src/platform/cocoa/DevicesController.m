//
//  DevicesController.m
//  Static
//
//  Created by Curtis Jones on 2010.01.02.
//  Copyright 2010 Curtis Jones. All rights reserved.
//

#import "DevicesController.h"
#import "../../core/core.h"
#import "../../device/device.h"
#import "../../driver/driver.h"





@interface __Device : NSObject
{
	device_t *mDevice;
}

@end

@implementation __Device
@end

@implementation DevicesController





#pragma mark -
#pragma mark NSObject

- (void)awakeFromNib
{
	
}





#pragma mark -
#pragma mark NSTableViewDataSource

/**
 * Returns the number of records managed for aTableView by the data source object.
 *
 */
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return coreobj()->device_count;
}

/**
 * Invoked by the table view to return the data object associated with the specified row and column.
 *
 */
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	device_t *device = core_device_get(0);
	
	if (device == nil)
		return @"Unknown";
	else
		return [NSString stringWithCString:device->desc.name encoding:NSASCIIStringEncoding];
}





#pragma mark -
#pragma mark NSTableViewDelegate

/**
 * Returns whether the cell at the specified row and column can be edited.
 *
 */
- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	return FALSE;
}

/**
 * Returns the height of the specified row.
 *
 */
- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return 20.;
}

/**
 * Returns whether the table view should allow selection of the specified row.
 *
 */
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
{
	return TRUE;
}

/**
 * Returns whether the specified table column can be selected.
 *
 */
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectTableColumn:(NSTableColumn *)aTableColumn
{
	return FALSE;
}

/**
 * Informs the delegate that the table view’s selection has changed.
 *
 */
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	NSLog(@"%s..", __PRETTY_FUNCTION__);
	
	device_t *device = core_device_get((uint32_t)[mTable selectedRow]);
	
	if (device == NULL)
		return;
	
	
}

@end
