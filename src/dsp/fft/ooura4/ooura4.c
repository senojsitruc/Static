/*
 *  ooura4.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "ooura4.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





#pragma mark -
#pragma mark private methods

static inline int __ooura4_feed (ooura4_t*, uint32_t*, double*);
static inline int __ooura4_reset (ooura4_t*);

static inline void __ooura4_cdft (int n, int isgn, double *a, int *ip, double *w);
static inline void __ooura4_rdft (int n, int isgn, double *a, int *ip, double *w);
static inline void __ooura4_ddct (int n, int isgn, double *a, int *ip, double *w);
static inline void __ooura4_ddst (int n, int isgn, double *a, int *ip, double *w);
static inline void __ooura4_dfct (int n, double *a, double *t, int *ip, double *w);
static inline void __ooura4_dfst (int n, double *a, double *t, int *ip, double *w);
static inline void __ooura4_makewt (int nw, int *ip, double *w);
static inline void __ooura4_makect (int nc, int *ip, double *c);
static inline void __ooura4_bitrv2 (int n, int *ip, double *a);
static inline void __ooura4_bitrv2conj (int n, int *ip, double *a);
static inline void __ooura4_cftfsub (int n, double *a, double *w);
static inline void __ooura4_cftbsub (int n, double *a, double *w);
static inline void __ooura4_cft1st (int n, double *a, double *w);
static inline void __ooura4_cftmdl (int n, int l, double *a, double *w);
static inline void __ooura4_rftfsub (int n, double *a, int nc, double *c);
static inline void __ooura4_rftbsub (int n, double *a, int nc, double *c);
static inline void __ooura4_dctsub (int n, double *a, int nc, double *c);
static inline void __ooura4_dstsub (int n, double *a, int nc, double *c);





#pragma mark -
#pragma mark structors

/**
 * "size" is the number of values to expect in the call to feed(); the number of I's and Q's,
 * combined.
 *
 */
int
ooura4_init (ooura4_t *ooura4, uint32_t size, fft_direction direction, fft_type type, opool_t *pool)
{
	int error;
	
	if (unlikely(ooura4 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura4_t");
	
	if (unlikely(size > 65536))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = fft_init((fft_t*)ooura4, size, direction, type, "dsp-fft-ooura4", (cobject_destroy_func)ooura4_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to fft_init, %d", error);
	
	if (unlikely(NULL == (ooura4->scratch = malloc(sizeof(int) * (size_t)(sqrt(size/2)+2)))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc(scratch), %s", strerror(errno));
	
	if (unlikely(NULL == (ooura4->sincos = malloc(sizeof(double) * (size/4)))))
		LOG_ERROR_AND_RETURN(-103, "failed to malloc(sincos), %s", strerror(errno));
	
	if (unlikely(NULL == (ooura4->data = malloc(sizeof(double) * size))))
		LOG_ERROR_AND_RETURN(-104, "failed to malloc(data), %s", strerror(errno));
	
	memset(ooura4->scratch, 0, sizeof(int) * (size_t)(sqrt(size/2)+2));
	memset(ooura4->sincos, 0, sizeof(double) * (size/4));
	memset(ooura4->data, 0, sizeof(double) * size);
	
	ooura4->fft.data_in = ooura4->data;
	ooura4->fft.data_out = ooura4->data;
	ooura4->fft.__feed = (__fft_feed_func)__ooura4_feed;
	ooura4->fft.__reset = (__fft_reset_func)__ooura4_reset;
	
	return 0;
}

int
ooura4_destroy (ooura4_t *ooura4)
{
	int error;
	
	if (unlikely(ooura4 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura4_t");
	
	if (ooura4->scratch != NULL) {
		free(ooura4->scratch);
		ooura4->scratch = NULL;
	}
	
	if (ooura4->sincos != NULL) {
		free(ooura4->sincos);
		ooura4->sincos = NULL;
	}
	
	if (ooura4->data != NULL) {
		free(ooura4->data);
		ooura4->data = NULL;
	}
	
	if (unlikely(0 != (error = fft_destroy((fft_t*)ooura4))))
		LOG_ERROR_AND_RETURN(-101, "failed to fft_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark run

/**
 *
 *
 */
static inline int
__ooura4_feed (ooura4_t *ooura4, uint32_t *size, double *data)
{
	size_t _size;
	
	if (unlikely(ooura4 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura4_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	_size = *size;
	
	if (unlikely(_size > ooura4->fft.size * sizeof(double)))
		LOG_ERROR_AND_RETURN(-4, "mismatched sizes (size=%lu, ooura4->fft.size=%lu)", _size, (ooura4->fft.size*sizeof(double)));
	
	memset(ooura4->data, 0, sizeof(double) * ooura4->fft.size);
	memcpy(ooura4->data, data, _size);
	
	// complex
	if (FFT_COMPLEX == ooura4->fft.type) {
		if (FFT_DIR_FORWARD == ooura4->fft.direction)
			__ooura4_cdft((int)ooura4->fft.size, -1, ooura4->data, ooura4->scratch, ooura4->sincos);
		else if (FFT_DIR_BACKWARD == ooura4->fft.direction)
			__ooura4_cdft((int)ooura4->fft.size, 1, ooura4->data, ooura4->scratch, ooura4->sincos);
	}
	
	// real
	else if (FFT_REAL == ooura4->fft.type) {
		if (FFT_DIR_FORWARD == ooura4->fft.direction)
			__ooura4_rdft((int)ooura4->fft.size, -1, ooura4->data, ooura4->scratch, ooura4->sincos);
		else if (FFT_DIR_BACKWARD == ooura4->fft.direction)
			__ooura4_rdft((int)ooura4->fft.size, 1, ooura4->data, ooura4->scratch, ooura4->sincos);
	}
	
	_size = (sizeof(double) * ooura4->fft.size);
	
	// the first shall become last and the last shall become first
	memcpy(data, ooura4->data+(ooura4->fft.size/2), _size / 2);
	memcpy(data+(ooura4->fft.size/2), ooura4->data, _size / 2);
	
	*size = (uint32_t)_size;
	
	return 0;
}

/**
 *
 *
 */
static inline int
__ooura4_reset (ooura4_t *ooura4)
{
	uint32_t size;
	
	if (unlikely(ooura4 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura4_t");
	
	size = ooura4->fft.size;
	
	memset(ooura4->scratch, 0, sizeof(int) * (size_t)(sqrt(size/2)+2));
	memset(ooura4->sincos, 0, sizeof(double) * (size/4));
	memset(ooura4->data, 0, sizeof(double) * size);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline ooura4_t*
ooura4_retain (ooura4_t *ooura4)
{
	if (unlikely(ooura4 == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null ooura4_t");
	
	return (ooura4_t*)fft_retain((fft_t*)ooura4);
}

/**
 *
 *
 */
inline void
ooura4_release (ooura4_t *ooura4)
{
	if (unlikely(ooura4 == NULL))
		LOG_ERROR_AND_RETURN(, "null ooura4_t");
	
	fft_release((fft_t*)ooura4);
}





#pragma mark -
#pragma mark ooura fft

/**
 *
 *
 */
static inline void
__ooura4_cdft (int n, int isgn, double *a, int *ip, double *w)
{
	if (n > (ip[0] << 2))
		__ooura4_makewt(n >> 2, ip, w);
	
	if (n > 4) {
		if (isgn >= 0) {
			__ooura4_bitrv2(n, ip + 2, a);
			__ooura4_cftfsub(n, a, w);
		} else {
			__ooura4_bitrv2conj(n, ip + 2, a);
			__ooura4_cftbsub(n, a, w);
		}
	} else if (n == 4)
		__ooura4_cftfsub(n, a, w);
}

/**
 *
 *
 */
static inline void
__ooura4_rdft (int n, int isgn, double *a, int *ip, double *w)
{
	int nw, nc;
	double xi;
	
	nw = ip[0];
	if (n > (nw << 2)) {
		nw = n >> 2;
		__ooura4_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > (nc << 2)) {
		nc = n >> 2;
		__ooura4_makect(nc, ip, w + nw);
	}
	if (isgn >= 0) {
		if (n > 4) {
			__ooura4_bitrv2(n, ip + 2, a);
			__ooura4_cftfsub(n, a, w);
			__ooura4_rftfsub(n, a, nc, w + nw);
		} else if (n == 4) {
			__ooura4_cftfsub(n, a, w);
		}
		xi = a[0] - a[1];
		a[0] += a[1];
		a[1] = xi;
	} else {
		a[1] = 0.5 * (a[0] - a[1]);
		a[0] -= a[1];
		if (n > 4) {
			__ooura4_rftbsub(n, a, nc, w + nw);
			__ooura4_bitrv2(n, ip + 2, a);
			__ooura4_cftbsub(n, a, w);
		} else if (n == 4) {
			__ooura4_cftfsub(n, a, w);
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura4_ddct (int n, int isgn, double *a, int *ip, double *w)
{
	int j, nw, nc;
	double xr;
	
	nw = ip[0];
	if (n > (nw << 2)) {
		nw = n >> 2;
		__ooura4_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > nc) {
		nc = n;
		__ooura4_makect(nc, ip, w + nw);
	}
	if (isgn < 0) {
		xr = a[n - 1];
		for (j = n - 2; j >= 2; j -= 2) {
			a[j + 1] = a[j] - a[j - 1];
			a[j] += a[j - 1];
		}
		a[1] = a[0] - xr;
		a[0] += xr;
		if (n > 4) {
			__ooura4_rftbsub(n, a, nc, w + nw);
			__ooura4_bitrv2(n, ip + 2, a);
			__ooura4_cftbsub(n, a, w);
		} else if (n == 4) {
			__ooura4_cftfsub(n, a, w);
		}
	}
	__ooura4_dctsub(n, a, nc, w + nw);
	if (isgn >= 0) {
		if (n > 4) {
			__ooura4_bitrv2(n, ip + 2, a);
			__ooura4_cftfsub(n, a, w);
			__ooura4_rftfsub(n, a, nc, w + nw);
		} else if (n == 4) {
			__ooura4_cftfsub(n, a, w);
		}
		xr = a[0] - a[1];
		a[0] += a[1];
		for (j = 2; j < n; j += 2) {
			a[j - 1] = a[j] - a[j + 1];
			a[j] += a[j + 1];
		}
		a[n - 1] = xr;
	}
}

/**
 *
 *
 */
static inline void
__ooura4_ddst (int n, int isgn, double *a, int *ip, double *w)
{
	int j, nw, nc;
	double xr;
	
	nw = ip[0];
	if (n > (nw << 2)) {
		nw = n >> 2;
		__ooura4_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > nc) {
		nc = n;
		__ooura4_makect(nc, ip, w + nw);
	}
	if (isgn < 0) {
		xr = a[n - 1];
		for (j = n - 2; j >= 2; j -= 2) {
			a[j + 1] = -a[j] - a[j - 1];
			a[j] -= a[j - 1];
		}
		a[1] = a[0] + xr;
		a[0] -= xr;
		if (n > 4) {
			__ooura4_rftbsub(n, a, nc, w + nw);
			__ooura4_bitrv2(n, ip + 2, a);
			__ooura4_cftbsub(n, a, w);
		} else if (n == 4) {
			__ooura4_cftfsub(n, a, w);
		}
	}
	__ooura4_dstsub(n, a, nc, w + nw);
	if (isgn >= 0) {
		if (n > 4) {
			__ooura4_bitrv2(n, ip + 2, a);
			__ooura4_cftfsub(n, a, w);
			__ooura4_rftfsub(n, a, nc, w + nw);
		} else if (n == 4) {
			__ooura4_cftfsub(n, a, w);
		}
		xr = a[0] - a[1];
		a[0] += a[1];
		for (j = 2; j < n; j += 2) {
			a[j - 1] = -a[j] - a[j + 1];
			a[j] -= a[j + 1];
		}
		a[n - 1] = -xr;
	}
}

/**
 *
 *
 */
static inline void
__ooura4_dfct (int n, double *a, double *t, int *ip, double *w)
{
	int j, k, l, m, mh, nw, nc;
	double xr, xi, yr, yi;
	
	nw = ip[0];
	if (n > (nw << 3)) {
		nw = n >> 3;
		__ooura4_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > (nc << 1)) {
		nc = n >> 1;
		__ooura4_makect(nc, ip, w + nw);
	}
	m = n >> 1;
	yi = a[m];
	xi = a[0] + a[n];
	a[0] -= a[n];
	t[0] = xi - yi;
	t[m] = xi + yi;
	if (n > 2) {
		mh = m >> 1;
		for (j = 1; j < mh; j++) {
			k = m - j;
			xr = a[j] - a[n - j];
			xi = a[j] + a[n - j];
			yr = a[k] - a[n - k];
			yi = a[k] + a[n - k];
			a[j] = xr;
			a[k] = yr;
			t[j] = xi - yi;
			t[k] = xi + yi;
		}
		t[mh] = a[mh] + a[n - mh];
		a[mh] -= a[n - mh];
		__ooura4_dctsub(m, a, nc, w + nw);
		if (m > 4) {
			__ooura4_bitrv2(m, ip + 2, a);
			__ooura4_cftfsub(m, a, w);
			__ooura4_rftfsub(m, a, nc, w + nw);
		} else if (m == 4) {
			__ooura4_cftfsub(m, a, w);
		}
		a[n - 1] = a[0] - a[1];
		a[1] = a[0] + a[1];
		for (j = m - 2; j >= 2; j -= 2) {
			a[2 * j + 1] = a[j] + a[j + 1];
			a[2 * j - 1] = a[j] - a[j + 1];
		}
		l = 2;
		m = mh;
		while (m >= 2) {
			__ooura4_dctsub(m, t, nc, w + nw);
			if (m > 4) {
				__ooura4_bitrv2(m, ip + 2, t);
				__ooura4_cftfsub(m, t, w);
				__ooura4_rftfsub(m, t, nc, w + nw);
			} else if (m == 4) {
				__ooura4_cftfsub(m, t, w);
			}
			a[n - l] = t[0] - t[1];
			a[l] = t[0] + t[1];
			k = 0;
			for (j = 2; j < m; j += 2) {
				k += l << 2;
				a[k - l] = t[j] - t[j + 1];
				a[k + l] = t[j] + t[j + 1];
			}
			l <<= 1;
			mh = m >> 1;
			for (j = 0; j < mh; j++) {
				k = m - j;
				t[j] = t[m + k] - t[m + j];
				t[k] = t[m + k] + t[m + j];
			}
			t[mh] = t[m + mh];
			m = mh;
		}
		a[l] = t[0];
		a[n] = t[2] - t[1];
		a[0] = t[2] + t[1];
	} else {
		a[1] = a[0];
		a[2] = t[0];
		a[0] = t[1];
	}
}

/**
 *
 *
 */
static inline void
__ooura4_dfst (int n, double *a, double *t, int *ip, double *w)
{
	int j, k, l, m, mh, nw, nc;
	double xr, xi, yr, yi;
	
	nw = ip[0];
	if (n > (nw << 3)) {
		nw = n >> 3;
		__ooura4_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > (nc << 1)) {
		nc = n >> 1;
		__ooura4_makect(nc, ip, w + nw);
	}
	if (n > 2) {
		m = n >> 1;
		mh = m >> 1;
		for (j = 1; j < mh; j++) {
			k = m - j;
			xr = a[j] + a[n - j];
			xi = a[j] - a[n - j];
			yr = a[k] + a[n - k];
			yi = a[k] - a[n - k];
			a[j] = xr;
			a[k] = yr;
			t[j] = xi + yi;
			t[k] = xi - yi;
		}
		t[0] = a[mh] - a[n - mh];
		a[mh] += a[n - mh];
		a[0] = a[m];
		__ooura4_dstsub(m, a, nc, w + nw);
		if (m > 4) {
			__ooura4_bitrv2(m, ip + 2, a);
			__ooura4_cftfsub(m, a, w);
			__ooura4_rftfsub(m, a, nc, w + nw);
		} else if (m == 4) {
			__ooura4_cftfsub(m, a, w);
		}
		a[n - 1] = a[1] - a[0];
		a[1] = a[0] + a[1];
		for (j = m - 2; j >= 2; j -= 2) {
			a[2 * j + 1] = a[j] - a[j + 1];
			a[2 * j - 1] = -a[j] - a[j + 1];
		}
		l = 2;
		m = mh;
		while (m >= 2) {
			__ooura4_dstsub(m, t, nc, w + nw);
			if (m > 4) {
				__ooura4_bitrv2(m, ip + 2, t);
				__ooura4_cftfsub(m, t, w);
				__ooura4_rftfsub(m, t, nc, w + nw);
			} else if (m == 4) {
				__ooura4_cftfsub(m, t, w);
			}
			a[n - l] = t[1] - t[0];
			a[l] = t[0] + t[1];
			k = 0;
			for (j = 2; j < m; j += 2) {
				k += l << 2;
				a[k - l] = -t[j] - t[j + 1];
				a[k + l] = t[j] - t[j + 1];
			}
			l <<= 1;
			mh = m >> 1;
			for (j = 1; j < mh; j++) {
				k = m - j;
				t[j] = t[m + k] + t[m + j];
				t[k] = t[m + k] - t[m + j];
			}
			t[0] = t[m + mh];
			m = mh;
		}
		a[l] = t[0];
	}
	a[0] = 0;
}

/**
 *
 *
 */
static inline void
__ooura4_makewt (int nw, int *ip, double *w)
{
	int j, nwh;
	double delta, x, y;
	
	ip[0] = nw;
	ip[1] = 1;
	if (nw > 2) {
		nwh = nw >> 1;
		delta = atan(1.0) / nwh;
		w[0] = 1;
		w[1] = 0;
		w[nwh] = cos(delta * nwh);
		w[nwh + 1] = w[nwh];
		if (nwh > 2) {
			for (j = 2; j < nwh; j += 2) {
				x = cos(delta * j);
				y = sin(delta * j);
				w[j] = x;
				w[j + 1] = y;
				w[nw - j] = y;
				w[nw - j + 1] = x;
			}
			__ooura4_bitrv2(nw, ip + 2, w);
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura4_makect (int nc, int *ip, double *c)
{
	int j, nch;
	double delta;
	
	ip[1] = nc;
	if (nc > 1) {
		nch = nc >> 1;
		delta = atan(1.0) / nch;
		c[0] = cos(delta * nch);
		c[nch] = 0.5 * c[0];
		for (j = 1; j < nch; j++) {
			c[j] = 0.5 * cos(delta * j);
			c[nc - j] = 0.5 * sin(delta * j);
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura4_bitrv2 (int n, int *ip, double *a)
{
	int j, j1, k, k1, l, m, m2;
	double xr, xi, yr, yi;
	
	ip[0] = 0;
	l = n;
	m = 1;
	while ((m << 3) < l) {
		l >>= 1;
		for (j = 0; j < m; j++) {
			ip[m + j] = ip[j] + l;
		}
		m <<= 1;
	}
	m2 = 2 * m;
	if ((m << 3) == l) {
		for (k = 0; k < m; k++) {
			for (j = 0; j < k; j++) {
				j1 = 2 * j + ip[k];
				k1 = 2 * k + ip[j];
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m2;
				k1 += 2 * m2;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m2;
				k1 -= m2;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m2;
				k1 += 2 * m2;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
			}
			j1 = 2 * k + m2 + ip[k];
			k1 = j1 + m2;
			xr = a[j1];
			xi = a[j1 + 1];
			yr = a[k1];
			yi = a[k1 + 1];
			a[j1] = yr;
			a[j1 + 1] = yi;
			a[k1] = xr;
			a[k1 + 1] = xi;
		}
	} else {
		for (k = 1; k < m; k++) {
			for (j = 0; j < k; j++) {
				j1 = 2 * j + ip[k];
				k1 = 2 * k + ip[j];
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m2;
				k1 += m2;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
			}
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura4_bitrv2conj (int n, int *ip, double *a)
{
	int j, j1, k, k1, l, m, m2;
	double xr, xi, yr, yi;
	
	ip[0] = 0;
	l = n;
	m = 1;
	while ((m << 3) < l) {
		l >>= 1;
		for (j = 0; j < m; j++) {
			ip[m + j] = ip[j] + l;
		}
		m <<= 1;
	}
	m2 = 2 * m;
	if ((m << 3) == l) {
		for (k = 0; k < m; k++) {
			for (j = 0; j < k; j++) {
				j1 = 2 * j + ip[k];
				k1 = 2 * k + ip[j];
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m2;
				k1 += 2 * m2;
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m2;
				k1 -= m2;
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m2;
				k1 += 2 * m2;
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
			}
			k1 = 2 * k + ip[k];
			a[k1 + 1] = -a[k1 + 1];
			j1 = k1 + m2;
			k1 = j1 + m2;
			xr = a[j1];
			xi = -a[j1 + 1];
			yr = a[k1];
			yi = -a[k1 + 1];
			a[j1] = yr;
			a[j1 + 1] = yi;
			a[k1] = xr;
			a[k1 + 1] = xi;
			k1 += m2;
			a[k1 + 1] = -a[k1 + 1];
		}
	} else {
		a[1] = -a[1];
		a[m2 + 1] = -a[m2 + 1];
		for (k = 1; k < m; k++) {
			for (j = 0; j < k; j++) {
				j1 = 2 * j + ip[k];
				k1 = 2 * k + ip[j];
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m2;
				k1 += m2;
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
			}
			k1 = 2 * k + ip[k];
			a[k1 + 1] = -a[k1 + 1];
			a[k1 + m2 + 1] = -a[k1 + m2 + 1];
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura4_cftfsub (int n, double *a, double *w)
{
	int j, j1, j2, j3, l;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	
	l = 2;
	if (n > 8) {
		__ooura4_cft1st(n, a, w);
		l = 8;
		while ((l << 2) < n) {
			__ooura4_cftmdl(n, l, a, w);
			l <<= 2;
		}
	}
	if ((l << 2) == n) {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			x0r = a[j] + a[j1];
			x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r;
			a[j + 1] = x0i + x2i;
			a[j2] = x0r - x2r;
			a[j2 + 1] = x0i - x2i;
			a[j1] = x1r - x3i;
			a[j1 + 1] = x1i + x3r;
			a[j3] = x1r + x3i;
			a[j3 + 1] = x1i - x3r;
		}
	} else {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			x0r = a[j] - a[j1];
			x0i = a[j + 1] - a[j1 + 1];
			a[j] += a[j1];
			a[j + 1] += a[j1 + 1];
			a[j1] = x0r;
			a[j1 + 1] = x0i;
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura4_cftbsub (int n, double *a, double *w)
{
	int j, j1, j2, j3, l;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	
	l = 2;
	if (n > 8) {
		__ooura4_cft1st(n, a, w);
		l = 8;
		while ((l << 2) < n) {
			__ooura4_cftmdl(n, l, a, w);
			l <<= 2;
		}
	}
	if ((l << 2) == n) {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			x0r = a[j] + a[j1];
			x0i = -a[j + 1] - a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = -a[j + 1] + a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r;
			a[j + 1] = x0i - x2i;
			a[j2] = x0r - x2r;
			a[j2 + 1] = x0i + x2i;
			a[j1] = x1r - x3i;
			a[j1 + 1] = x1i - x3r;
			a[j3] = x1r + x3i;
			a[j3 + 1] = x1i + x3r;
		}
	} else {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			x0r = a[j] - a[j1];
			x0i = -a[j + 1] + a[j1 + 1];
			a[j] += a[j1];
			a[j + 1] = -a[j + 1] - a[j1 + 1];
			a[j1] = x0r;
			a[j1 + 1] = x0i;
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura4_cft1st (int n, double *a, double *w)
{
	int j, k1, k2;
	double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	
	x0r = a[0] + a[2];
	x0i = a[1] + a[3];
	x1r = a[0] - a[2];
	x1i = a[1] - a[3];
	x2r = a[4] + a[6];
	x2i = a[5] + a[7];
	x3r = a[4] - a[6];
	x3i = a[5] - a[7];
	a[0] = x0r + x2r;
	a[1] = x0i + x2i;
	a[4] = x0r - x2r;
	a[5] = x0i - x2i;
	a[2] = x1r - x3i;
	a[3] = x1i + x3r;
	a[6] = x1r + x3i;
	a[7] = x1i - x3r;
	wk1r = w[2];
	x0r = a[8] + a[10];
	x0i = a[9] + a[11];
	x1r = a[8] - a[10];
	x1i = a[9] - a[11];
	x2r = a[12] + a[14];
	x2i = a[13] + a[15];
	x3r = a[12] - a[14];
	x3i = a[13] - a[15];
	a[8] = x0r + x2r;
	a[9] = x0i + x2i;
	a[12] = x2i - x0i;
	a[13] = x0r - x2r;
	x0r = x1r - x3i;
	x0i = x1i + x3r;
	a[10] = wk1r * (x0r - x0i);
	a[11] = wk1r * (x0r + x0i);
	x0r = x3i + x1r;
	x0i = x3r - x1i;
	a[14] = wk1r * (x0i - x0r);
	a[15] = wk1r * (x0i + x0r);
	k1 = 0;
	for (j = 16; j < n; j += 16) {
		k1 += 2;
		k2 = 2 * k1;
		wk2r = w[k1];
		wk2i = w[k1 + 1];
		wk1r = w[k2];
		wk1i = w[k2 + 1];
		wk3r = wk1r - 2 * wk2i * wk1i;
		wk3i = 2 * wk2i * wk1r - wk1i;
		x0r = a[j] + a[j + 2];
		x0i = a[j + 1] + a[j + 3];
		x1r = a[j] - a[j + 2];
		x1i = a[j + 1] - a[j + 3];
		x2r = a[j + 4] + a[j + 6];
		x2i = a[j + 5] + a[j + 7];
		x3r = a[j + 4] - a[j + 6];
		x3i = a[j + 5] - a[j + 7];
		a[j] = x0r + x2r;
		a[j + 1] = x0i + x2i;
		x0r -= x2r;
		x0i -= x2i;
		a[j + 4] = wk2r * x0r - wk2i * x0i;
		a[j + 5] = wk2r * x0i + wk2i * x0r;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		a[j + 2] = wk1r * x0r - wk1i * x0i;
		a[j + 3] = wk1r * x0i + wk1i * x0r;
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		a[j + 6] = wk3r * x0r - wk3i * x0i;
		a[j + 7] = wk3r * x0i + wk3i * x0r;
		wk1r = w[k2 + 2];
		wk1i = w[k2 + 3];
		wk3r = wk1r - 2 * wk2r * wk1i;
		wk3i = 2 * wk2r * wk1r - wk1i;
		x0r = a[j + 8] + a[j + 10];
		x0i = a[j + 9] + a[j + 11];
		x1r = a[j + 8] - a[j + 10];
		x1i = a[j + 9] - a[j + 11];
		x2r = a[j + 12] + a[j + 14];
		x2i = a[j + 13] + a[j + 15];
		x3r = a[j + 12] - a[j + 14];
		x3i = a[j + 13] - a[j + 15];
		a[j + 8] = x0r + x2r;
		a[j + 9] = x0i + x2i;
		x0r -= x2r;
		x0i -= x2i;
		a[j + 12] = -wk2i * x0r - wk2r * x0i;
		a[j + 13] = -wk2i * x0i + wk2r * x0r;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		a[j + 10] = wk1r * x0r - wk1i * x0i;
		a[j + 11] = wk1r * x0i + wk1i * x0r;
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		a[j + 14] = wk3r * x0r - wk3i * x0i;
		a[j + 15] = wk3r * x0i + wk3i * x0r;
	}
}

/**
 *
 *
 */
static inline void
__ooura4_cftmdl (int n, int l, double *a, double *w)
{
	int j, j1, j2, j3, k, k1, k2, m, m2;
	double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	
	m = l << 2;
	for (j = 0; j < l; j += 2) {
		j1 = j + l;
		j2 = j1 + l;
		j3 = j2 + l;
		x0r = a[j] + a[j1];
		x0i = a[j + 1] + a[j1 + 1];
		x1r = a[j] - a[j1];
		x1i = a[j + 1] - a[j1 + 1];
		x2r = a[j2] + a[j3];
		x2i = a[j2 + 1] + a[j3 + 1];
		x3r = a[j2] - a[j3];
		x3i = a[j2 + 1] - a[j3 + 1];
		a[j] = x0r + x2r;
		a[j + 1] = x0i + x2i;
		a[j2] = x0r - x2r;
		a[j2 + 1] = x0i - x2i;
		a[j1] = x1r - x3i;
		a[j1 + 1] = x1i + x3r;
		a[j3] = x1r + x3i;
		a[j3 + 1] = x1i - x3r;
	}
	wk1r = w[2];
	for (j = m; j < l + m; j += 2) {
		j1 = j + l;
		j2 = j1 + l;
		j3 = j2 + l;
		x0r = a[j] + a[j1];
		x0i = a[j + 1] + a[j1 + 1];
		x1r = a[j] - a[j1];
		x1i = a[j + 1] - a[j1 + 1];
		x2r = a[j2] + a[j3];
		x2i = a[j2 + 1] + a[j3 + 1];
		x3r = a[j2] - a[j3];
		x3i = a[j2 + 1] - a[j3 + 1];
		a[j] = x0r + x2r;
		a[j + 1] = x0i + x2i;
		a[j2] = x2i - x0i;
		a[j2 + 1] = x0r - x2r;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		a[j1] = wk1r * (x0r - x0i);
		a[j1 + 1] = wk1r * (x0r + x0i);
		x0r = x3i + x1r;
		x0i = x3r - x1i;
		a[j3] = wk1r * (x0i - x0r);
		a[j3 + 1] = wk1r * (x0i + x0r);
	}
	k1 = 0;
	m2 = 2 * m;
	for (k = m2; k < n; k += m2) {
		k1 += 2;
		k2 = 2 * k1;
		wk2r = w[k1];
		wk2i = w[k1 + 1];
		wk1r = w[k2];
		wk1i = w[k2 + 1];
		wk3r = wk1r - 2 * wk2i * wk1i;
		wk3i = 2 * wk2i * wk1r - wk1i;
		for (j = k; j < l + k; j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			x0r = a[j] + a[j1];
			x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r;
			a[j + 1] = x0i + x2i;
			x0r -= x2r;
			x0i -= x2i;
			a[j2] = wk2r * x0r - wk2i * x0i;
			a[j2 + 1] = wk2r * x0i + wk2i * x0r;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			a[j1] = wk1r * x0r - wk1i * x0i;
			a[j1 + 1] = wk1r * x0i + wk1i * x0r;
			x0r = x1r + x3i;
			x0i = x1i - x3r;
			a[j3] = wk3r * x0r - wk3i * x0i;
			a[j3 + 1] = wk3r * x0i + wk3i * x0r;
		}
		wk1r = w[k2 + 2];
		wk1i = w[k2 + 3];
		wk3r = wk1r - 2 * wk2r * wk1i;
		wk3i = 2 * wk2r * wk1r - wk1i;
		for (j = k + m; j < l + (k + m); j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			x0r = a[j] + a[j1];
			x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r;
			a[j + 1] = x0i + x2i;
			x0r -= x2r;
			x0i -= x2i;
			a[j2] = -wk2i * x0r - wk2r * x0i;
			a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			a[j1] = wk1r * x0r - wk1i * x0i;
			a[j1 + 1] = wk1r * x0i + wk1i * x0r;
			x0r = x1r + x3i;
			x0i = x1i - x3r;
			a[j3] = wk3r * x0r - wk3i * x0i;
			a[j3 + 1] = wk3r * x0i + wk3i * x0r;
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura4_rftfsub (int n, double *a, int nc, double *c)
{
	int j, k, kk, ks, m;
	double wkr, wki, xr, xi, yr, yi;
	
	m = n >> 1;
	ks = 2 * nc / m;
	kk = 0;
	for (j = 2; j < m; j += 2) {
		k = n - j;
		kk += ks;
		wkr = 0.5 - c[nc - kk];
		wki = c[kk];
		xr = a[j] - a[k];
		xi = a[j + 1] + a[k + 1];
		yr = wkr * xr - wki * xi;
		yi = wkr * xi + wki * xr;
		a[j] -= yr;
		a[j + 1] -= yi;
		a[k] += yr;
		a[k + 1] -= yi;
	}
}

/**
 *
 *
 */
static inline void
__ooura4_rftbsub (int n, double *a, int nc, double *c)
{
	int j, k, kk, ks, m;
	double wkr, wki, xr, xi, yr, yi;
	
	a[1] = -a[1];
	m = n >> 1;
	ks = 2 * nc / m;
	kk = 0;
	for (j = 2; j < m; j += 2) {
		k = n - j;
		kk += ks;
		wkr = 0.5 - c[nc - kk];
		wki = c[kk];
		xr = a[j] - a[k];
		xi = a[j + 1] + a[k + 1];
		yr = wkr * xr + wki * xi;
		yi = wkr * xi - wki * xr;
		a[j] -= yr;
		a[j + 1] = yi - a[j + 1];
		a[k] += yr;
		a[k + 1] = yi - a[k + 1];
	}
	a[m + 1] = -a[m + 1];
}

/**
 *
 *
 */
static inline void
__ooura4_dctsub (int n, double *a, int nc, double *c)
{
	int j, k, kk, ks, m;
	double wkr, wki, xr;
	
	m = n >> 1;
	ks = nc / n;
	kk = 0;
	for (j = 1; j < m; j++) {
		k = n - j;
		kk += ks;
		wkr = c[kk] - c[nc - kk];
		wki = c[kk] + c[nc - kk];
		xr = wki * a[j] - wkr * a[k];
		a[j] = wkr * a[j] + wki * a[k];
		a[k] = xr;
	}
	a[m] *= c[0];
}

/**
 *
 *
 */
static inline void
__ooura4_dstsub (int n, double *a, int nc, double *c)
{
	int j, k, kk, ks, m;
	double wkr, wki, xr;
	
	m = n >> 1;
	ks = nc / n;
	kk = 0;
	for (j = 1; j < m; j++) {
		k = n - j;
		kk += ks;
		wkr = c[kk] - c[nc - kk];
		wki = c[kk] + c[nc - kk];
		xr = wki * a[k] - wkr * a[j];
		a[k] = wkr * a[k] + wki * a[j];
		a[j] = xr;
	}
	a[m] *= c[0];
}
