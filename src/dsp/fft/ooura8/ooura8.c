/*
 *  ooura8.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "ooura8.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





#pragma mark -
#pragma mark private methods

static inline int __ooura8_feed (ooura8_t*, uint32_t*, double*);
static inline int __ooura8_reset (ooura8_t*);

static inline void __ooura8_cdft (int n, int isgn, double *a, int *ip, double *w);
static inline void __ooura8_rdft (int n, int isgn, double *a, int *ip, double *w);
static inline void __ooura8_ddct (int n, int isgn, double *a, int *ip, double *w);
static inline void __ooura8_ddst (int n, int isgn, double *a, int *ip, double *w);
static inline void __ooura8_dfct (int n, double *a, double *t, int *ip, double *w);
static inline void __ooura8_dfst (int n, double *a, double *t, int *ip, double *w);
static inline void __ooura8_makewt (int nw, int *ip, double *w);
static inline void __ooura8_makect (int nc, int *ip, double *c);
static inline void __ooura8_bitrv2 (int n, int *ip, double *a);
static inline void __ooura8_bitrv2conj (int n, int *ip, double *a);
static inline void __ooura8_cftfsub (int n, double *a, double *w);
static inline void __ooura8_cftbsub (int n, double *a, double *w);
static inline void __ooura8_cft1st (int n, double *a, double *w);
static inline void __ooura8_cftmdl (int n, int l, double *a, double *w);
static inline void __ooura8_rftfsub (int n, double *a, int nc, double *c);
static inline void __ooura8_rftbsub (int n, double *a, int nc, double *c);
static inline void __ooura8_dctsub (int n, double *a, int nc, double *c);
static inline void __ooura8_dstsub (int n, double *a, int nc, double *c);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
ooura8_init (ooura8_t *ooura8, uint32_t size, fft_direction direction, fft_type type, opool_t *pool)
{
	int error;
	
	if (unlikely(ooura8 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura8_t");
	
	if (unlikely(size > 10000))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = fft_init((fft_t*)ooura8, size, direction, type, "dsp-fft-ooura8", (cobject_destroy_func)ooura8_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to fft_init, %d", error);
	
	if (unlikely(NULL == (ooura8->scratch = malloc(sizeof(int) * (size_t)(sqrt(size/2)+2)))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc(scratch), %s", strerror(errno));
	
	if (unlikely(NULL == (ooura8->sincos = malloc(sizeof(double) * (size/4)))))
		LOG_ERROR_AND_RETURN(-103, "failed to malloc(sincos), %s", strerror(errno));
	
	if (unlikely(NULL == (ooura8->data = malloc(sizeof(double) * size))))
		LOG_ERROR_AND_RETURN(-104, "failed to malloc(data), %s", strerror(errno));
	
	memset(ooura8->scratch, 0, sizeof(int) * (size_t)(sqrt(size/2)+2));
	memset(ooura8->sincos, 0, sizeof(double) * (size/4));
	memset(ooura8->data, 0, sizeof(double) * size);
	
	ooura8->fft.data_in = ooura8->data;
	ooura8->fft.data_out = ooura8->data;
	ooura8->fft.__feed = (__fft_feed_func)__ooura8_feed;
	ooura8->fft.__reset = (__fft_reset_func)__ooura8_reset;
	
	return 0;
}

int
ooura8_destroy (ooura8_t *ooura8)
{
	int error;
	
	if (unlikely(ooura8 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura8_t");
	
	if (ooura8->scratch != NULL) {
		free(ooura8->scratch);
		ooura8->scratch = NULL;
	}
	
	if (ooura8->sincos != NULL) {
		free(ooura8->sincos);
		ooura8->sincos = NULL;
	}
	
	if (ooura8->data != NULL) {
		free(ooura8->data);
		ooura8->data = NULL;
	}
	
	if (unlikely(0 != (error = fft_destroy((fft_t*)ooura8))))
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
__ooura8_feed (ooura8_t *ooura8, uint32_t *size, double *data)
{
	size_t _size;
	
	if (unlikely(ooura8 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura8_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	if (unlikely(*size != ooura8->fft.size * sizeof(double)))
		LOG_ERROR_AND_RETURN(-4, "mismatched sizes (size=%u, ooura8->fft.size=%lu)", *size, (ooura8->fft.size*sizeof(double)));
	
	memset(ooura8->data, 0, sizeof(double) * ooura8->fft.size);
	memcpy(ooura8->data, data, *size);
	
	// complex
	if (FFT_COMPLEX == ooura8->fft.type) {
		if (FFT_DIR_FORWARD == ooura8->fft.direction)
			__ooura8_cdft((int)ooura8->fft.size, -1, ooura8->data, ooura8->scratch, ooura8->sincos);
		else if (FFT_DIR_BACKWARD == ooura8->fft.direction)
			__ooura8_cdft((int)ooura8->fft.size, 1, ooura8->data, ooura8->scratch, ooura8->sincos);
	}
	
	// real
	else if (FFT_REAL == ooura8->fft.type) {
		if (FFT_DIR_FORWARD == ooura8->fft.direction)
			__ooura8_rdft((int)ooura8->fft.size, -1, ooura8->data, ooura8->scratch, ooura8->sincos);
		else if (FFT_DIR_BACKWARD == ooura8->fft.direction)
			__ooura8_rdft((int)ooura8->fft.size, 1, ooura8->data, ooura8->scratch, ooura8->sincos);
	}
	
	_size = (sizeof(double) * ooura8->fft.size);
	
	// the first shall become last and the last shall become first
	memcpy(data, ooura8->data+ooura8->fft.size/2, _size / 2);
	memcpy(data+ooura8->fft.size/2, ooura8->data, _size / 2);
	
	*size = (uint32_t)_size;
	
	return 0;
}

/**
 *
 *
 */
static inline int
__ooura8_reset (ooura8_t *ooura8)
{
	uint32_t size;
	
	if (unlikely(ooura8 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ooura8_t");
	
	size = ooura8->fft.size;
	
	memset(ooura8->scratch, 0, sizeof(int) * (size_t)(sqrt(size/2)+2));
	memset(ooura8->sincos, 0, sizeof(double) * (size/4));
	memset(ooura8->data, 0, sizeof(double) * size);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline ooura8_t*
ooura8_retain (ooura8_t *ooura8)
{
	if (unlikely(ooura8 == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null ooura8_t");
	
	return (ooura8_t*)fft_retain((fft_t*)ooura8);
}

/**
 *
 *
 */
inline void
ooura8_release (ooura8_t *ooura8)
{
	if (unlikely(ooura8 == NULL))
		LOG_ERROR_AND_RETURN(, "null ooura8_t");
	
	fft_release((fft_t*)ooura8);
}





#pragma mark -
#pragma mark ooura fft

/**
 *
 *
 */
static inline void
__ooura8_cdft (int n, int isgn, double *a, int *ip, double *w)
{
	if (n > (ip[0] << 2))
		__ooura8_makewt(n >> 2, ip, w);
	
	if (n > 4) {
		if (isgn >= 0) {
			__ooura8_bitrv2(n, ip + 2, a);
			__ooura8_cftfsub(n, a, w);
		} else {
			__ooura8_bitrv2conj(n, ip + 2, a);
			__ooura8_cftbsub(n, a, w);
		}
	} else if (n == 4)
		__ooura8_cftfsub(n, a, w);
}

/**
 *
 *
 */
static inline void
__ooura8_rdft (int n, int isgn, double *a, int *ip, double *w)
{
	int nw, nc;
	double xi;
	
	nw = ip[0];
	if (n > (nw << 2)) {
		nw = n >> 2;
		__ooura8_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > (nc << 2)) {
		nc = n >> 2;
		__ooura8_makect(nc, ip, w + nw);
	}
	if (isgn >= 0) {
		if (n > 4) {
			__ooura8_bitrv2(n, ip + 2, a);
			__ooura8_cftfsub(n, a, w);
			__ooura8_rftfsub(n, a, nc, w + nw);
		} else if (n == 4) {
			__ooura8_cftfsub(n, a, w);
		}
		xi = a[0] - a[1];
		a[0] += a[1];
		a[1] = xi;
	} else {
		a[1] = 0.5 * (a[0] - a[1]);
		a[0] -= a[1];
		if (n > 4) {
			__ooura8_rftbsub(n, a, nc, w + nw);
			__ooura8_bitrv2(n, ip + 2, a);
			__ooura8_cftbsub(n, a, w);
		} else if (n == 4) {
			__ooura8_cftfsub(n, a, w);
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura8_ddct (int n, int isgn, double *a, int *ip, double *w)
{
	int j, nw, nc;
	double xr;
	
	nw = ip[0];
	if (n > (nw << 2)) {
		nw = n >> 2;
		__ooura8_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > nc) {
		nc = n;
		__ooura8_makect(nc, ip, w + nw);
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
			__ooura8_rftbsub(n, a, nc, w + nw);
			__ooura8_bitrv2(n, ip + 2, a);
			__ooura8_cftbsub(n, a, w);
		} else if (n == 4) {
			__ooura8_cftfsub(n, a, w);
		}
	}
	__ooura8_dctsub(n, a, nc, w + nw);
	if (isgn >= 0) {
		if (n > 4) {
			__ooura8_bitrv2(n, ip + 2, a);
			__ooura8_cftfsub(n, a, w);
			__ooura8_rftfsub(n, a, nc, w + nw);
		} else if (n == 4) {
			__ooura8_cftfsub(n, a, w);
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
__ooura8_ddst (int n, int isgn, double *a, int *ip, double *w)
{
	int j, nw, nc;
	double xr;
	
	nw = ip[0];
	if (n > (nw << 2)) {
		nw = n >> 2;
		__ooura8_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > nc) {
		nc = n;
		__ooura8_makect(nc, ip, w + nw);
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
			__ooura8_rftbsub(n, a, nc, w + nw);
			__ooura8_bitrv2(n, ip + 2, a);
			__ooura8_cftbsub(n, a, w);
		} else if (n == 4) {
			__ooura8_cftfsub(n, a, w);
		}
	}
	__ooura8_dstsub(n, a, nc, w + nw);
	if (isgn >= 0) {
		if (n > 4) {
			__ooura8_bitrv2(n, ip + 2, a);
			__ooura8_cftfsub(n, a, w);
			__ooura8_rftfsub(n, a, nc, w + nw);
		} else if (n == 4) {
			__ooura8_cftfsub(n, a, w);
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
__ooura8_dfct (int n, double *a, double *t, int *ip, double *w)
{
	int j, k, l, m, mh, nw, nc;
	double xr, xi, yr, yi;
	
	nw = ip[0];
	if (n > (nw << 3)) {
		nw = n >> 3;
		__ooura8_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > (nc << 1)) {
		nc = n >> 1;
		__ooura8_makect(nc, ip, w + nw);
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
		__ooura8_dctsub(m, a, nc, w + nw);
		if (m > 4) {
			__ooura8_bitrv2(m, ip + 2, a);
			__ooura8_cftfsub(m, a, w);
			__ooura8_rftfsub(m, a, nc, w + nw);
		} else if (m == 4) {
			__ooura8_cftfsub(m, a, w);
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
			__ooura8_dctsub(m, t, nc, w + nw);
			if (m > 4) {
				__ooura8_bitrv2(m, ip + 2, t);
				__ooura8_cftfsub(m, t, w);
				__ooura8_rftfsub(m, t, nc, w + nw);
			} else if (m == 4) {
				__ooura8_cftfsub(m, t, w);
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
__ooura8_dfst (int n, double *a, double *t, int *ip, double *w)
{
	int j, k, l, m, mh, nw, nc;
	double xr, xi, yr, yi;
	
	nw = ip[0];
	if (n > (nw << 3)) {
		nw = n >> 3;
		__ooura8_makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > (nc << 1)) {
		nc = n >> 1;
		__ooura8_makect(nc, ip, w + nw);
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
		__ooura8_dstsub(m, a, nc, w + nw);
		if (m > 4) {
			__ooura8_bitrv2(m, ip + 2, a);
			__ooura8_cftfsub(m, a, w);
			__ooura8_rftfsub(m, a, nc, w + nw);
		} else if (m == 4) {
			__ooura8_cftfsub(m, a, w);
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
			__ooura8_dstsub(m, t, nc, w + nw);
			if (m > 4) {
				__ooura8_bitrv2(m, ip + 2, t);
				__ooura8_cftfsub(m, t, w);
				__ooura8_rftfsub(m, t, nc, w + nw);
			} else if (m == 4) {
				__ooura8_cftfsub(m, t, w);
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
__ooura8_makewt (int nw, int *ip, double *w)
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
			for (j = nwh - 2; j >= 2; j -= 2) {
				x = w[2 * j];
				y = w[2 * j + 1];
				w[nwh + j] = x;
				w[nwh + j + 1] = y;
			}
			__ooura8_bitrv2(nw, ip + 2, w);
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura8_makect (int nc, int *ip, double *c)
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
__ooura8_bitrv2 (int n, int *ip, double *a)
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
__ooura8_bitrv2conj (int n, int *ip, double *a)
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
__ooura8_cftfsub (int n, double *a, double *w)
{
	int j, j1, j2, j3, l;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	
	l = 2;
	if (n >= 16) {
		__ooura8_cft1st(n, a, w);
		l = 16;
		while ((l << 3) <= n) {
			__ooura8_cftmdl(n, l, a, w);
			l <<= 3;
		}
	}
	if ((l << 1) < n) {
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
	} else if ((l << 1) == n) {
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
__ooura8_cftbsub (int n, double *a, double *w)
{
	int j, j1, j2, j3, j4, j5, j6, j7, l;
	double wn4r, x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i, 
	y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i, 
	y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i;
	
	l = 2;
	if (n > 16) {
		__ooura8_cft1st(n, a, w);
		l = 16;
		while ((l << 3) < n) {
			__ooura8_cftmdl(n, l, a, w);
			l <<= 3;
		}
	}
	if ((l << 2) < n) {
		wn4r = w[2];
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			j4 = j3 + l;
			j5 = j4 + l;
			j6 = j5 + l;
			j7 = j6 + l;
			x0r = a[j] + a[j1];
			x0i = -a[j + 1] - a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = -a[j + 1] + a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			y0r = x0r + x2r;
			y0i = x0i - x2i;
			y2r = x0r - x2r;
			y2i = x0i + x2i;
			y1r = x1r - x3i;
			y1i = x1i - x3r;
			y3r = x1r + x3i;
			y3i = x1i + x3r;
			x0r = a[j4] + a[j5];
			x0i = a[j4 + 1] + a[j5 + 1];
			x1r = a[j4] - a[j5];
			x1i = a[j4 + 1] - a[j5 + 1];
			x2r = a[j6] + a[j7];
			x2i = a[j6 + 1] + a[j7 + 1];
			x3r = a[j6] - a[j7];
			x3i = a[j6 + 1] - a[j7 + 1];
			y4r = x0r + x2r;
			y4i = x0i + x2i;
			y6r = x0r - x2r;
			y6i = x0i - x2i;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			x2r = x1r + x3i;
			x2i = x1i - x3r;
			y5r = wn4r * (x0r - x0i);
			y5i = wn4r * (x0r + x0i);
			y7r = wn4r * (x2r - x2i);
			y7i = wn4r * (x2r + x2i);
			a[j1] = y1r + y5r;
			a[j1 + 1] = y1i - y5i;
			a[j5] = y1r - y5r;
			a[j5 + 1] = y1i + y5i;
			a[j3] = y3r - y7i;
			a[j3 + 1] = y3i - y7r;
			a[j7] = y3r + y7i;
			a[j7 + 1] = y3i + y7r;
			a[j] = y0r + y4r;
			a[j + 1] = y0i - y4i;
			a[j4] = y0r - y4r;
			a[j4 + 1] = y0i + y4i;
			a[j2] = y2r - y6i;
			a[j2 + 1] = y2i - y6r;
			a[j6] = y2r + y6i;
			a[j6 + 1] = y2i + y6r;
		}
	} else if ((l << 2) == n) {
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
__ooura8_cft1st (int n, double *a, double *w)
{
	int j, k1;
	double wn4r, wtmp, wk1r, wk1i, wk2r, wk2i, wk3r, wk3i, 
	wk4r, wk4i, wk5r, wk5i, wk6r, wk6i, wk7r, wk7i;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i, 
	y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i, 
	y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i;
	
	wn4r = w[2];
	x0r = a[0] + a[2];
	x0i = a[1] + a[3];
	x1r = a[0] - a[2];
	x1i = a[1] - a[3];
	x2r = a[4] + a[6];
	x2i = a[5] + a[7];
	x3r = a[4] - a[6];
	x3i = a[5] - a[7];
	y0r = x0r + x2r;
	y0i = x0i + x2i;
	y2r = x0r - x2r;
	y2i = x0i - x2i;
	y1r = x1r - x3i;
	y1i = x1i + x3r;
	y3r = x1r + x3i;
	y3i = x1i - x3r;
	x0r = a[8] + a[10];
	x0i = a[9] + a[11];
	x1r = a[8] - a[10];
	x1i = a[9] - a[11];
	x2r = a[12] + a[14];
	x2i = a[13] + a[15];
	x3r = a[12] - a[14];
	x3i = a[13] - a[15];
	y4r = x0r + x2r;
	y4i = x0i + x2i;
	y6r = x0r - x2r;
	y6i = x0i - x2i;
	x0r = x1r - x3i;
	x0i = x1i + x3r;
	x2r = x1r + x3i;
	x2i = x1i - x3r;
	y5r = wn4r * (x0r - x0i);
	y5i = wn4r * (x0r + x0i);
	y7r = wn4r * (x2r - x2i);
	y7i = wn4r * (x2r + x2i);
	a[2] = y1r + y5r;
	a[3] = y1i + y5i;
	a[10] = y1r - y5r;
	a[11] = y1i - y5i;
	a[6] = y3r - y7i;
	a[7] = y3i + y7r;
	a[14] = y3r + y7i;
	a[15] = y3i - y7r;
	a[0] = y0r + y4r;
	a[1] = y0i + y4i;
	a[8] = y0r - y4r;
	a[9] = y0i - y4i;
	a[4] = y2r - y6i;
	a[5] = y2i + y6r;
	a[12] = y2r + y6i;
	a[13] = y2i - y6r;
	if (n > 16) {
		wk1r = w[4];
		wk1i = w[5];
		x0r = a[16] + a[18];
		x0i = a[17] + a[19];
		x1r = a[16] - a[18];
		x1i = a[17] - a[19];
		x2r = a[20] + a[22];
		x2i = a[21] + a[23];
		x3r = a[20] - a[22];
		x3i = a[21] - a[23];
		y0r = x0r + x2r;
		y0i = x0i + x2i;
		y2r = x0r - x2r;
		y2i = x0i - x2i;
		y1r = x1r - x3i;
		y1i = x1i + x3r;
		y3r = x1r + x3i;
		y3i = x1i - x3r;
		x0r = a[24] + a[26];
		x0i = a[25] + a[27];
		x1r = a[24] - a[26];
		x1i = a[25] - a[27];
		x2r = a[28] + a[30];
		x2i = a[29] + a[31];
		x3r = a[28] - a[30];
		x3i = a[29] - a[31];
		y4r = x0r + x2r;
		y4i = x0i + x2i;
		y6r = x0r - x2r;
		y6i = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		x2r = x1r + x3i;
		x2i = x3r - x1i;
		y5r = wk1i * x0r - wk1r * x0i;
		y5i = wk1i * x0i + wk1r * x0r;
		y7r = wk1r * x2r + wk1i * x2i;
		y7i = wk1r * x2i - wk1i * x2r;
		x0r = wk1r * y1r - wk1i * y1i;
		x0i = wk1r * y1i + wk1i * y1r;
		a[18] = x0r + y5r;
		a[19] = x0i + y5i;
		a[26] = y5i - x0i;
		a[27] = x0r - y5r;
		x0r = wk1i * y3r - wk1r * y3i;
		x0i = wk1i * y3i + wk1r * y3r;
		a[22] = x0r - y7r;
		a[23] = x0i + y7i;
		a[30] = y7i - x0i;
		a[31] = x0r + y7r;
		a[16] = y0r + y4r;
		a[17] = y0i + y4i;
		a[24] = y4i - y0i;
		a[25] = y0r - y4r;
		x0r = y2r - y6i;
		x0i = y2i + y6r;
		a[20] = wn4r * (x0r - x0i);
		a[21] = wn4r * (x0i + x0r);
		x0r = y6r - y2i;
		x0i = y2r + y6i;
		a[28] = wn4r * (x0r - x0i);
		a[29] = wn4r * (x0i + x0r);
		k1 = 4;
		for (j = 32; j < n; j += 16) {
			k1 += 4;
			wk1r = w[k1];
			wk1i = w[k1 + 1];
			wk2r = w[k1 + 2];
			wk2i = w[k1 + 3];
			wtmp = 2 * wk2i;
			wk3r = wk1r - wtmp * wk1i;
			wk3i = wtmp * wk1r - wk1i;
			wk4r = 1 - wtmp * wk2i;
			wk4i = wtmp * wk2r;
			wtmp = 2 * wk4i;
			wk5r = wk3r - wtmp * wk1i;
			wk5i = wtmp * wk1r - wk3i;
			wk6r = wk2r - wtmp * wk2i;
			wk6i = wtmp * wk2r - wk2i;
			wk7r = wk1r - wtmp * wk3i;
			wk7i = wtmp * wk3r - wk1i;
			x0r = a[j] + a[j + 2];
			x0i = a[j + 1] + a[j + 3];
			x1r = a[j] - a[j + 2];
			x1i = a[j + 1] - a[j + 3];
			x2r = a[j + 4] + a[j + 6];
			x2i = a[j + 5] + a[j + 7];
			x3r = a[j + 4] - a[j + 6];
			x3i = a[j + 5] - a[j + 7];
			y0r = x0r + x2r;
			y0i = x0i + x2i;
			y2r = x0r - x2r;
			y2i = x0i - x2i;
			y1r = x1r - x3i;
			y1i = x1i + x3r;
			y3r = x1r + x3i;
			y3i = x1i - x3r;
			x0r = a[j + 8] + a[j + 10];
			x0i = a[j + 9] + a[j + 11];
			x1r = a[j + 8] - a[j + 10];
			x1i = a[j + 9] - a[j + 11];
			x2r = a[j + 12] + a[j + 14];
			x2i = a[j + 13] + a[j + 15];
			x3r = a[j + 12] - a[j + 14];
			x3i = a[j + 13] - a[j + 15];
			y4r = x0r + x2r;
			y4i = x0i + x2i;
			y6r = x0r - x2r;
			y6i = x0i - x2i;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			x2r = x1r + x3i;
			x2i = x1i - x3r;
			y5r = wn4r * (x0r - x0i);
			y5i = wn4r * (x0r + x0i);
			y7r = wn4r * (x2r - x2i);
			y7i = wn4r * (x2r + x2i);
			x0r = y1r + y5r;
			x0i = y1i + y5i;
			a[j + 2] = wk1r * x0r - wk1i * x0i;
			a[j + 3] = wk1r * x0i + wk1i * x0r;
			x0r = y1r - y5r;
			x0i = y1i - y5i;
			a[j + 10] = wk5r * x0r - wk5i * x0i;
			a[j + 11] = wk5r * x0i + wk5i * x0r;
			x0r = y3r - y7i;
			x0i = y3i + y7r;
			a[j + 6] = wk3r * x0r - wk3i * x0i;
			a[j + 7] = wk3r * x0i + wk3i * x0r;
			x0r = y3r + y7i;
			x0i = y3i - y7r;
			a[j + 14] = wk7r * x0r - wk7i * x0i;
			a[j + 15] = wk7r * x0i + wk7i * x0r;
			a[j] = y0r + y4r;
			a[j + 1] = y0i + y4i;
			x0r = y0r - y4r;
			x0i = y0i - y4i;
			a[j + 8] = wk4r * x0r - wk4i * x0i;
			a[j + 9] = wk4r * x0i + wk4i * x0r;
			x0r = y2r - y6i;
			x0i = y2i + y6r;
			a[j + 4] = wk2r * x0r - wk2i * x0i;
			a[j + 5] = wk2r * x0i + wk2i * x0r;
			x0r = y2r + y6i;
			x0i = y2i - y6r;
			a[j + 12] = wk6r * x0r - wk6i * x0i;
			a[j + 13] = wk6r * x0i + wk6i * x0r;
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura8_cftmdl (int n, int l, double *a, double *w)
{
	int j, j1, j2, j3, j4, j5, j6, j7, k, k1, m;
	double wn4r, wtmp, wk1r, wk1i, wk2r, wk2i, wk3r, wk3i, 
	wk4r, wk4i, wk5r, wk5i, wk6r, wk6i, wk7r, wk7i;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i, 
	y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i, 
	y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i;
	
	m = l << 3;
	wn4r = w[2];
	for (j = 0; j < l; j += 2) {
		j1 = j + l;
		j2 = j1 + l;
		j3 = j2 + l;
		j4 = j3 + l;
		j5 = j4 + l;
		j6 = j5 + l;
		j7 = j6 + l;
		x0r = a[j] + a[j1];
		x0i = a[j + 1] + a[j1 + 1];
		x1r = a[j] - a[j1];
		x1i = a[j + 1] - a[j1 + 1];
		x2r = a[j2] + a[j3];
		x2i = a[j2 + 1] + a[j3 + 1];
		x3r = a[j2] - a[j3];
		x3i = a[j2 + 1] - a[j3 + 1];
		y0r = x0r + x2r;
		y0i = x0i + x2i;
		y2r = x0r - x2r;
		y2i = x0i - x2i;
		y1r = x1r - x3i;
		y1i = x1i + x3r;
		y3r = x1r + x3i;
		y3i = x1i - x3r;
		x0r = a[j4] + a[j5];
		x0i = a[j4 + 1] + a[j5 + 1];
		x1r = a[j4] - a[j5];
		x1i = a[j4 + 1] - a[j5 + 1];
		x2r = a[j6] + a[j7];
		x2i = a[j6 + 1] + a[j7 + 1];
		x3r = a[j6] - a[j7];
		x3i = a[j6 + 1] - a[j7 + 1];
		y4r = x0r + x2r;
		y4i = x0i + x2i;
		y6r = x0r - x2r;
		y6i = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		x2r = x1r + x3i;
		x2i = x1i - x3r;
		y5r = wn4r * (x0r - x0i);
		y5i = wn4r * (x0r + x0i);
		y7r = wn4r * (x2r - x2i);
		y7i = wn4r * (x2r + x2i);
		a[j1] = y1r + y5r;
		a[j1 + 1] = y1i + y5i;
		a[j5] = y1r - y5r;
		a[j5 + 1] = y1i - y5i;
		a[j3] = y3r - y7i;
		a[j3 + 1] = y3i + y7r;
		a[j7] = y3r + y7i;
		a[j7 + 1] = y3i - y7r;
		a[j] = y0r + y4r;
		a[j + 1] = y0i + y4i;
		a[j4] = y0r - y4r;
		a[j4 + 1] = y0i - y4i;
		a[j2] = y2r - y6i;
		a[j2 + 1] = y2i + y6r;
		a[j6] = y2r + y6i;
		a[j6 + 1] = y2i - y6r;
	}
	if (m < n) {
		wk1r = w[4];
		wk1i = w[5];
		for (j = m; j < l + m; j += 2) {
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			j4 = j3 + l;
			j5 = j4 + l;
			j6 = j5 + l;
			j7 = j6 + l;
			x0r = a[j] + a[j1];
			x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			y0r = x0r + x2r;
			y0i = x0i + x2i;
			y2r = x0r - x2r;
			y2i = x0i - x2i;
			y1r = x1r - x3i;
			y1i = x1i + x3r;
			y3r = x1r + x3i;
			y3i = x1i - x3r;
			x0r = a[j4] + a[j5];
			x0i = a[j4 + 1] + a[j5 + 1];
			x1r = a[j4] - a[j5];
			x1i = a[j4 + 1] - a[j5 + 1];
			x2r = a[j6] + a[j7];
			x2i = a[j6 + 1] + a[j7 + 1];
			x3r = a[j6] - a[j7];
			x3i = a[j6 + 1] - a[j7 + 1];
			y4r = x0r + x2r;
			y4i = x0i + x2i;
			y6r = x0r - x2r;
			y6i = x0i - x2i;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			x2r = x1r + x3i;
			x2i = x3r - x1i;
			y5r = wk1i * x0r - wk1r * x0i;
			y5i = wk1i * x0i + wk1r * x0r;
			y7r = wk1r * x2r + wk1i * x2i;
			y7i = wk1r * x2i - wk1i * x2r;
			x0r = wk1r * y1r - wk1i * y1i;
			x0i = wk1r * y1i + wk1i * y1r;
			a[j1] = x0r + y5r;
			a[j1 + 1] = x0i + y5i;
			a[j5] = y5i - x0i;
			a[j5 + 1] = x0r - y5r;
			x0r = wk1i * y3r - wk1r * y3i;
			x0i = wk1i * y3i + wk1r * y3r;
			a[j3] = x0r - y7r;
			a[j3 + 1] = x0i + y7i;
			a[j7] = y7i - x0i;
			a[j7 + 1] = x0r + y7r;
			a[j] = y0r + y4r;
			a[j + 1] = y0i + y4i;
			a[j4] = y4i - y0i;
			a[j4 + 1] = y0r - y4r;
			x0r = y2r - y6i;
			x0i = y2i + y6r;
			a[j2] = wn4r * (x0r - x0i);
			a[j2 + 1] = wn4r * (x0i + x0r);
			x0r = y6r - y2i;
			x0i = y2r + y6i;
			a[j6] = wn4r * (x0r - x0i);
			a[j6 + 1] = wn4r * (x0i + x0r);
		}
		k1 = 4;
		for (k = 2 * m; k < n; k += m) {
			k1 += 4;
			wk1r = w[k1];
			wk1i = w[k1 + 1];
			wk2r = w[k1 + 2];
			wk2i = w[k1 + 3];
			wtmp = 2 * wk2i;
			wk3r = wk1r - wtmp * wk1i;
			wk3i = wtmp * wk1r - wk1i;
			wk4r = 1 - wtmp * wk2i;
			wk4i = wtmp * wk2r;
			wtmp = 2 * wk4i;
			wk5r = wk3r - wtmp * wk1i;
			wk5i = wtmp * wk1r - wk3i;
			wk6r = wk2r - wtmp * wk2i;
			wk6i = wtmp * wk2r - wk2i;
			wk7r = wk1r - wtmp * wk3i;
			wk7i = wtmp * wk3r - wk1i;
			for (j = k; j < l + k; j += 2) {
				j1 = j + l;
				j2 = j1 + l;
				j3 = j2 + l;
				j4 = j3 + l;
				j5 = j4 + l;
				j6 = j5 + l;
				j7 = j6 + l;
				x0r = a[j] + a[j1];
				x0i = a[j + 1] + a[j1 + 1];
				x1r = a[j] - a[j1];
				x1i = a[j + 1] - a[j1 + 1];
				x2r = a[j2] + a[j3];
				x2i = a[j2 + 1] + a[j3 + 1];
				x3r = a[j2] - a[j3];
				x3i = a[j2 + 1] - a[j3 + 1];
				y0r = x0r + x2r;
				y0i = x0i + x2i;
				y2r = x0r - x2r;
				y2i = x0i - x2i;
				y1r = x1r - x3i;
				y1i = x1i + x3r;
				y3r = x1r + x3i;
				y3i = x1i - x3r;
				x0r = a[j4] + a[j5];
				x0i = a[j4 + 1] + a[j5 + 1];
				x1r = a[j4] - a[j5];
				x1i = a[j4 + 1] - a[j5 + 1];
				x2r = a[j6] + a[j7];
				x2i = a[j6 + 1] + a[j7 + 1];
				x3r = a[j6] - a[j7];
				x3i = a[j6 + 1] - a[j7 + 1];
				y4r = x0r + x2r;
				y4i = x0i + x2i;
				y6r = x0r - x2r;
				y6i = x0i - x2i;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				x2r = x1r + x3i;
				x2i = x1i - x3r;
				y5r = wn4r * (x0r - x0i);
				y5i = wn4r * (x0r + x0i);
				y7r = wn4r * (x2r - x2i);
				y7i = wn4r * (x2r + x2i);
				x0r = y1r + y5r;
				x0i = y1i + y5i;
				a[j1] = wk1r * x0r - wk1i * x0i;
				a[j1 + 1] = wk1r * x0i + wk1i * x0r;
				x0r = y1r - y5r;
				x0i = y1i - y5i;
				a[j5] = wk5r * x0r - wk5i * x0i;
				a[j5 + 1] = wk5r * x0i + wk5i * x0r;
				x0r = y3r - y7i;
				x0i = y3i + y7r;
				a[j3] = wk3r * x0r - wk3i * x0i;
				a[j3 + 1] = wk3r * x0i + wk3i * x0r;
				x0r = y3r + y7i;
				x0i = y3i - y7r;
				a[j7] = wk7r * x0r - wk7i * x0i;
				a[j7 + 1] = wk7r * x0i + wk7i * x0r;
				a[j] = y0r + y4r;
				a[j + 1] = y0i + y4i;
				x0r = y0r - y4r;
				x0i = y0i - y4i;
				a[j4] = wk4r * x0r - wk4i * x0i;
				a[j4 + 1] = wk4r * x0i + wk4i * x0r;
				x0r = y2r - y6i;
				x0i = y2i + y6r;
				a[j2] = wk2r * x0r - wk2i * x0i;
				a[j2 + 1] = wk2r * x0i + wk2i * x0r;
				x0r = y2r + y6i;
				x0i = y2i - y6r;
				a[j6] = wk6r * x0r - wk6i * x0i;
				a[j6 + 1] = wk6r * x0i + wk6i * x0r;
			}
		}
	}
}

/**
 *
 *
 */
static inline void
__ooura8_rftfsub (int n, double *a, int nc, double *c)
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
__ooura8_rftbsub (int n, double *a, int nc, double *c)
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
__ooura8_dctsub (int n, double *a, int nc, double *c)
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
__ooura8_dstsub (int n, double *a, int nc, double *c)
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
