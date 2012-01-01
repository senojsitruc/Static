/*
 *  main.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.15.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "../../../core/core.h"
#include "../../../misc/logger.h"
#include "chebyshev.h"

/**
 *
 *
 */
int
main (int argc, char **argv)
{
	int i, error;
	chebyshev_t *chebyshev;
	
	if (0 != (error = memlock_setup()))
		LOG_ERROR_AND_RETURN(-1, "failed to memlock_init, %d", error);
	
	if (0 != (error = core_init()))
		LOG_ERROR_AND_RETURN(-2, "failed to core_init, %d", error);
	
	if (0 != (error = core_dsp_filter_chebyshev(&chebyshev, 0.45, CHEBYSHEV_LOW_PASS, 0.5, 6, 100)))
		LOG_ERROR_AND_RETURN(-3, "failed to core_dsp_filter_chebyshev, %d", error);
	
	for (i = 0; i <= 23; ++i)
		LOG3("coef_a[%02d] = %lf, coef_b[%02d] = %lf", i, chebyshev->coef_a[i], i, chebyshev->coef_b[i]);
	
	/*
	if (0 != (error = core_dsp_filter_chebyshev(&chebyshev, 0.1, CHEBYSHEV_HIGH_PASS, 10., 4, 100)))
		LOG_ERROR_AND_RETURN(-3, "failed to core_dsp_filter_chebyshev, %d", error);
	
	for (i = 0; i <= 23; ++i)
		LOG3("coef_a[%02d] = %lf, coef_b[%02d] = %lf", i, chebyshev->coef_a[i], i, chebyshev->coef_b[i]);
	*/
	
	return EXIT_SUCCESS;
}
