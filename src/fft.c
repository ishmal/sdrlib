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
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include "sdrlib.h"
#include "fft.h"
#include "private.h"

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
    fft->plan  = fftw_plan_dft_1d(N, fft->in, fft->out, FFTW_FORWARD, FFTW_MEASURE);
    int psSize = N;
    fft->spectrum = (unsigned int *) malloc(psSize * sizeof(unsigned int));
    fft->inPtr = 0;
    fft->skipCounter = 0;
    fft->threshold = N * 10;
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





typedef struct
{
    float complex W;
    float complex x;
} Bin;


typedef struct QueueItem QueueItem;

struct QueueItem
{
    float complex v;
    QueueItem *next;
};



typedef struct SlidingDft SlidingDft;

struct SlidingDft
{
    int N;
    int size;
    float Fs;
    QueueItem *queue;
    QueueItem *head;
    Bin bins[];
};


void sdftSetFreqs(SlidingDft *obj, float loFreq, float hiFreq);

SlidingDft *sdftCreate(int N, int size, float loFreq, float hiFreq, float sampleRate)
{
    SlidingDft *obj = (SlidingDft *) malloc(sizeof(SlidingDft) + size * sizeof(Bin));
    if (!obj)
        return NULL;
    memset(obj, 0, sizeof(SlidingDft));
    obj->N = N;
    obj->size = size;
    obj->Fs = sampleRate;
    int queueSize = N * sizeof(QueueItem);
    obj->queue = (QueueItem *) malloc(queueSize);
    if (!obj->queue)
        {
        free(obj);
        return NULL;
        }
    memset(obj->queue, 0, queueSize);
    for (int i=0 ; i < N-1 ; i++)
        obj->queue[i].next = &(obj->queue[i+1]);
    obj->queue[N-1].next = obj->head;
    obj->head = obj->queue;
    sdftSetFreqs(obj, loFreq, hiFreq);
    return obj;
}

void sdftDelete(SlidingDft *obj)
{
    if (obj)
        {
        free(obj->queue);
        free(obj);
        }
}

void sdftSetFreqs(SlidingDft *obj, float loFreq, float hiFreq)
{
    int size = obj->size;
    float Fs = obj->Fs;
    float deltaFreq = (hiFreq - loFreq) / size;
    float f = loFreq;
    Bin *bin = obj->bins;
    while (size--)
        {
        float omega = TWOPI * f / Fs;
        bin->W = cos(omega) + sin(omega)*I;
        bin->x = 0.0;
        }
}

void sdftUpdate(SlidingDft *obj, float complex *sample, int len)
{
    int size     = obj->size;
    Bin *bin     = obj->bins;
    QueueItem *q = obj->head;
    
    while (len--)
        {
        float complex v = *sample++;
        float complex diff = v - q->v;
        q->v = v;
        q = q->next;
        int count = size;
        while (count--)
            {
            bin->x += diff * bin->W;
            bin++;
            }
        }
        
    obj->head = q;
}


void sdftGetPowerSpectrum(SlidingDft *obj, unsigned int *out)
{
    int size = obj->size;
    Bin *bin = obj->bins;
    while (size--)
        {
        *out++ = (unsigned int)(20.0 * fasterlog2(1.0 + cabsf(bin->x)));
        bin++;
        }

}


