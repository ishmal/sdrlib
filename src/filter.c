/**
 * Various filter definitions and implementations.
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

#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include "private.h"
#include "filter.h"




Fir *firCreate(int size)
{
    Fir *fir = (Fir *)malloc(sizeof(Fir));
    if (!fir)
        return NULL;
    fir->size       = size;
    fir->coeffs     = (float *)malloc(size * sizeof(float));
    fir->delayLine  = (float *)malloc(size * sizeof(float));
    if (!fir->coeffs || !fir->delayLine)
        {
        free(fir->coeffs);
        free(fir->delayLine);
        free(fir);
        return NULL;
        }
    int i = 0;
    for ( ; i<size ; i++)
        {
        fir->coeffs[i] = 0.0;
        fir->delayLine[i] = 0.0;
        }
    fir->delayIndex = 0;
    return fir;
}

FirC *firCreateC(int size)
{
    FirC *fir = (FirC *)malloc(sizeof(FirC));
    if (!fir)
        return NULL;
    fir->size       = size;
    fir->coeffs     = (float *)malloc(size * sizeof(float));
    fir->delayLine  = (complex *)malloc(size * sizeof(complex));
    if (!fir->coeffs || !fir->delayLine)
        {
        free(fir->coeffs);
        free(fir->delayLine);
        free(fir);
        return NULL;
        }
    int i = 0;
    for ( ; i<size ; i++)
        {
        fir->coeffs[i] = 0.0;
        fir->delayLine[i] = 0.0;
        }
    fir->delayIndex = 0;
    return fir;
}


void firDelete(Fir *fir)
{
    if (fir)
        {
        free(fir->coeffs);
        free(fir->delayLine);
        free(fir);
        }
}

void firDeleteC(FirC *fir)
{
    if (fir)
        {
        free(fir->coeffs);
        free(fir->delayLine);
        free(fir);
        }
}


float firUpdate(Fir *fir, float sample)
{
    float *delayLine = fir->delayLine;
    int delayIndex = fir->delayIndex;
    delayLine[delayIndex] = sample;
    float sum = 0.0;
    //walk the coefficients from first to last
    //and the delay line from newest to oldest
    int idx = delayIndex;
    float *coeff = fir->coeffs;
    int size = fir->size;
    int count = size;
    while (count--)
        {
        float v = delayLine[idx];
        idx--;
        if (idx < 0)
            idx = size - 1;
        sum += v * (*coeff++);
        }
    fir->delayIndex = (delayIndex + 1) % size;
    return sum;
}




complex firUpdateC(FirC *fir, complex sample)
{
    complex *delayLine = fir->delayLine;
    int delayIndex = fir->delayIndex;
    delayLine[delayIndex] = sample;
    complex sum = 0.0;
    //walk the coefficients from first to last
    //and the delay line from newest to oldest
    int idx = delayIndex;
    float *coeff = fir->coeffs;
    int size = fir->size;
    int count = size;
    while (count--)
        {
        complex v = delayLine[idx];
        idx--;
        if (idx < 0)
            idx = size - 1;
        sum += v * (*coeff++);
        }
    fir->delayIndex = (delayIndex + 1) % size;
    return sum;
}


static void windowize(int size, float *coeffs, int windowType)
{
    int i = 0;
    switch (windowType)
        {
        case W_HAMMING: 
            {
            for ( ; i<size ; i++)
                coeffs[i] *= 0.54 - 0.46 * cos(TWOPI * i / (size-1));
            break;
            }
        case W_HANN: 
            {
            for ( ; i<size ; i++)
                coeffs[i] *= 0.5 - 0.5 * cos(TWOPI * i / (size-1));
            break;
            }
        case W_BLACKMAN: 
            {
            for ( ; i<size ; i++)
                coeffs[i] *= 
                    (
                    0.42 -
                    0.5 * cos(TWOPI * i / (size-1)) +
                    0.08 * cos(4.0 * PI * i / (size-1))
                    );
            break;
            }
        default: 
            {
            }
        }
}


static void normalize(int size, float *coeffs)
{
    float sum = 0.0;
    int i = 0;
    for ( ; i < size ; i++)
        sum += coeffs[i];
    float scale = 1.0 / sum;
    for (i=0 ; i < size ; i++)
        sum *= scale;
    
}



void firLPCoeffs(int size, float *coeffs, float cutoffFreq, float sampleRate)
{
    float omega = 2.0 * PI * cutoffFreq / sampleRate;
    int center = (size - 1) / 2;
    int i = 0;
    for ( ; i < size ; i++)
        coeffs[i] = (i == center) ? omega / PI : sin(omega * i) / (PI * i);
}

Fir *firLP(int size, float cutoffFreq, float sampleRate, int windowType)
{
    Fir *fir = firCreate(size);
    firLPCoeffs(size, fir->coeffs, cutoffFreq, sampleRate);
    windowize(size, fir->coeffs, windowType);
    normalize(size, fir->coeffs);
    return fir;
}

FirC *firLPC(int size, float cutoffFreq, float sampleRate, int windowType)
{
    FirC *fir = firCreateC(size);
    firLPCoeffs(size, fir->coeffs, cutoffFreq, sampleRate);
    windowize(size, fir->coeffs, windowType);
    normalize(size, fir->coeffs);
    return fir;
}



