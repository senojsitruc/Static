/*
 *  fileout.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.19.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  Dump the data at a current point in the dspchain to a file. This does not modify the data in
 *  any way and it attempts to format (tabulate) it in such a way that it is easily readable.
 *
 */

#ifndef __DSP_FILEOUT_H__
#define __DSP_FILEOUT_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// dspfileout
//
struct dspfileout
{
	dsp_t dsp;                           // parent class
	
	dsp_datatype type;                   // data type
	char path[100];                      // file path
	int fd;                              // file descriptor
	char *line;                          // line buffer
	uint32_t width;                      // line width
};
typedef struct dspfileout dspfileout_t;





/**
 *
 */
int dspfileout_init (dspfileout_t*, dsp_datatype, char*, opool_t*);

/**
 *
 */
int dspfileout_destroy (dspfileout_t*);





/**
 *
 */
int dspfileout_feed (dspfileout_t*, uint32_t*, void*);

/**
 *
 */
int dspfileout_reset (dspfileout_t*);





/**
 *
 */
dspfileout_t* dspfileout_retain (dspfileout_t*);

/**
 *
 */
void dspfileout_release (dspfileout_t*);

#endif /* __DSP_FILEOUT_H__ */
