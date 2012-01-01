//
//  main.m
//  Static
//
//  Created by Curtis Jones on 2009.12.18.
//  Copyright 2009 Curtis Jones. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <stdlib.h>
#import <time.h>

int
main(int argc, char *argv[])
{
	srandom((unsigned int)time(0));
	
	return NSApplicationMain(argc, (const char **)argv);
}
