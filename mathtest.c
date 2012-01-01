/*
 *  mathtest.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.19.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

int
main (int argc, char **argv)
{
	double complex c1, c2, c3;
	
	c1 = 1;
	c2 = cexp((I * -2.0 * M_PI * 750000) / 66666666);
	c3 = c1 * c2;
	
	printf("c1 = (%f, %f)\n", creal(c1), cimag(c1));
	printf("c2 = (%f, %f)\n", creal(c2), cimag(c2));
	printf("c3 = (%f, %f)\n", creal(c3), cimag(c3));
	
	for (int i = 0; i < 10; ++i) {
		c1 *= c2;
		printf("c1 = (%f, %f)\n", creal(c1), cimag(c1));
	}
	
	return EXIT_SUCCESS;
}
