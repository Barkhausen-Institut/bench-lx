/*
 * fix_fft_org.c
 */

#include "fix_fft.h"
#include "precalc.h"

extern short Sinewave[N_WAVE-N_WAVE/4];



static inline short FIX_MPY(short a, short b)
{
	/* shift right one less bit (i.e. 15-1) */
	int c = ((int)a * (int)b) >> 14;
	/* last bit shifted out = rounding-bit */
	b = c & 0x01;
	/* last shift + rounding bit */
	a = (c >> 1) + b;
	return a;
}


int fix_fft_original(short fr[], short fi[], short m, short inverse)
{
	int mr, nn, i, j, l, k, istep, n, scale, shift;
	short qr, qi, tr, ti, wr, wi;

	n = 1 << m;

	/* max FFT size = N_WAVE */
	if (n > N_WAVE)
		return -1;

	mr = 0;
	nn = n - 1;
	scale = 0;

	shift = SHIFT;

	/* decimation in time - re-order data */
	for (m=1; m<=nn; ++m) {
		l = n;
		do {
			l >>= 1;
		} while (mr+l > nn);
		mr = (mr & (l-1)) + l;

		if (mr <= m)
			continue;
		tr = fr[m];
		fr[m] = fr[mr];
		fr[mr] = tr;
		ti = fi[m];
		fi[m] = fi[mr];
		fi[mr] = ti;
	}


	l = 1;
	k = LOG2_N_WAVE-1;
	while (l < n) {
		/* no variable scaling
		if (inverse) {
			// variable scaling, depending upon data
			shift = 0;
			for (i=0; i<n; ++i) {
				j = fr[i];
				if (j < 0)
					j = -j;
				m = fi[i];
				if (m < 0)
					m = -m;
				if (j > 16383 || m > 16383) {
					shift = 1;
					break;
				}
			}
			if (shift)
				++scale;
		} else {

			//  fixed scaling, for proper normalization --
			//  there will be log2(n) passes, so this results
			//  in an overall factor of 1/n, distributed to
			//  maximize arithmetic accuracy.

			shift = 1;
		}*/
		/*
		  it may not be obvious, but the shift will be
		  performed on each data point exactly once,
		  during this pass.
		*/

		istep = l << 1;
		for (m=0; m<l; ++m) {
			j = m << k;
			/* 0 <= j < N_WAVE/2 */
			wr =  Sinewave[j+N_WAVE/4];
			wi = -Sinewave[j];

			if (inverse)
				wi = -wi;
			if (shift) {
				wr >>= 1;
				wi >>= 1;
			}

			for (i=m; i<n; i+=istep) {
				j = i + l;

				tr = FIX_MPY(wr,fr[j]) - FIX_MPY(wi,fi[j]);
				ti = FIX_MPY(wr,fi[j]) + FIX_MPY(wi,fr[j]);
				qr = fr[i];
				qi = fi[i];
				if (shift) {
					qr >>= 1;
					qi >>= 1;
				}
				fr[j] = qr - tr;
				fi[j] = qi - ti;
				fr[i] = qr + tr;
				fi[i] = qi + ti;
			}
		}

		--k;
		l = istep;
	}
	return scale;
}



