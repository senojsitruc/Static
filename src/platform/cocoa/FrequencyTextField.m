//
//  FrequencyTextField.m
//  Static
//
//  Created by Curtis Jones on 2010.01.01.
//  Copyright 2010 Curtis Jones. All rights reserved.
//

#import "FrequencyTextField.h"

@implementation FrequencyTextField

- (void)keyDown:(NSEvent *)event
{
	char const *c = NULL;
	
	c = [[event charactersIgnoringModifiers] UTF8String];
	
	if (c != NULL && (*c >= 0x48 || *c <= 0x57))
		[super keyDown:event];
}

@end
