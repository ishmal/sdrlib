/**
 * FFT definitions and implementations.
 * 
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2013 Bob Jamison
 * 
 *  This file is part of the SdrLib library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <sdrlib.h>
#include <inttypes.h>
#include <math.h>

#include "fft.h"

/**
 * Create a new Fft instance.
 * @return a new Fft instance
 */  
Fft *fftCreate(int N)
{
    Fft *fft = (Fft *)malloc(sizeof(Fft));
    if (!fft)
        return fft;
    fft->N     = N;
    fft->in    = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fft->out   = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fft->plan  = fftw_plan_dft_1d(N, fft->in, fft->out, FFTW_FORWARD, FFTW_ESTIMATE);
    int psSize = N;
    fft->spectrum = (unsigned int *) malloc(psSize * sizeof(unsigned int));
    fft->inPtr = 0;
    fft->skipCounter = 0;
    fft->threshold = N * 5;
    return fft;
}

/**
 * Delete an Fft instance, stopping
 * any processing and freeing any resources.
 * @param fft an Fft instance.
 */   
void fftDelete(Fft *fft)
{
    if (!fft)
        return;
    fftw_destroy_plan(fft->plan);
    free(fft->in);
    free(fft->out);
    free(fft->spectrum);
    free(fft);
}


static inline float 
fasterlog2 (float x)
{
  union { float f; uint32_t i; } vx = { x };
  float y = vx.i;
  y *= 1.1920928955078125e-7f;
  return y - 126.94269504f;
}



void fftUpdate(Fft *fft, float complex *inbuf, int count, FftOutputFunc *func, void *context)
{
    float complex *in = inbuf;
    fftw_complex  *fftwin = fft->in;
    int N     = fft->N;
    int inPtr = fft->inPtr;
    while (count--)
        {
        if ((fft->skipCounter++) < fft->threshold)
            continue;
        fftwin[inPtr++] = *in++;
        if (inPtr >= N)
            {
            inPtr = 0;
            fftw_execute(fft->plan);
            unsigned int *ps = fft->spectrum;
            int half = N>>1;
            fftw_complex *lower = fft->out;
            fftw_complex *upper = fft->out + half;
            int count = half;
            while (count--)
                {
                *ps++ = (unsigned int)(20.0 * fasterlog2(1.0 + cabsf(*upper++)));
                }
            count = half;
            while (count--)
                {
                *ps++ = (unsigned int)(20.0 * fasterlog2(1.0 + cabsf(*lower++)));
                }
            func(fft->spectrum, N, context);
            fft->skipCounter = 0;
            }
        }
    fft->inPtr = inPtr;
}

