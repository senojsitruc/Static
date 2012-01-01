/*
 *  dump.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "dump.h"
#include <stdio.h>

/**
 *
 *
 */
void
hexdump (uint8_t *buf, int len)
{
  int i, j, k;
  
  printf("     -------------------------------------------------------------------------------\n");
  
  for (i = 0; i < len;) {
    printf("     ");
    
    for (j = i; j < i + 8 && j < len; j++)
      printf("%02x ", (unsigned char)buf[j]);
		
    // if at this point we have reached the end of the packet data, we need to
    // pad this last line such that it becomes even with the rest of the lines.
    if (j >= len - 1) {
      for (k = len % 16; k < 8; k++)
        printf("   ");
    }
    
    printf("  ");
    
    for (j = i + 8; j < i + 16 && j < len; j++)
      printf("%02x ", (unsigned char)buf[j]);
		
    // if at this point we have reached the end of the packet data, we need to
    // pad this last line such that it becomes even with the rest of the lines.
    if (j >= len - 1) {
      for (k = 16; k > 8 && k > len % 16; k--)
        printf("   ");
    }
    
    printf("  |  ");
    
    for (j = i; j < i + 16 && j < len; j++) {
      if ((int)buf[j] >= 32 && (int)buf[j] <= 126)
        printf("%c", (unsigned char)buf[j]);
      else
        printf(".");
    }
		
    printf("\n");
    i += 16;
  }
  
  printf("     -------------------------------------------------------------------------------\n");
}
