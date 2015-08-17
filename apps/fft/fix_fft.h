/*
 * fix_fft.h
 */

#ifndef FIX_FFT_H
#define FIX_FFT_H

#define FFT_SIZE    128
#define LOG_2_FFT   7

//turn on variable scaling
#define SHIFT       1

#define N_WAVE      1024    /* full length of Sinewave[] */
#define LOG2_N_WAVE 10      /* log2(N_WAVE) */

#define SHUFFLE_N_128_SIZE 112

#ifdef __cplusplus
extern "C" {
#endif

//function prototypes
int fix_fft_original(short fr[], short fi[], short m, short inverse);

#ifdef __cplusplus
}
#endif

#endif /* FIX_FFT_H */
