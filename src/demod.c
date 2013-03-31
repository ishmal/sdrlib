/**
 * Various demodulation modes
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
#include <string.h>
#include "demod.h"
#include "private.h"


static void amDemodulate(Demodulator *dem, float complex *data, int size, DemodOutputFunc *func, void *context)
{
    int bufPtr = dem->bufPtr;
    float *buf = dem->outBuf;
    float complex lastVal = dem->lastVal;
    while (size--)
        {
        float complex prod = *data * conj(lastVal);
        lastVal = *data++;
        buf[bufPtr++] = cabsf(prod);
        if (bufPtr >= DEMOD_BUFSIZE)
            {
            int allocSize = bufPtr * sizeof(float);
            float *audiobuf = (float *)malloc(allocSize);
            memcpy(audiobuf, buf, allocSize);
            func(audiobuf, bufPtr, context); 
            bufPtr = 0;
            }
        }
    dem->lastVal = lastVal;
    dem->bufPtr  = bufPtr;
}


Demodulator *demodAmCreate()
{
    Demodulator *dem = (Demodulator *)smalloc(sizeof(Demodulator));
    if (!dem)
        return NULL;
    dem->bufPtr  = 0;
    dem->lastVal = 0;
    dem->update  = amDemodulate;
    return dem;
}

static void fmDemodulate(Demodulator *dem, float complex *data, int size, DemodOutputFunc *func, void *context)
{
    int bufPtr = dem->bufPtr;
    float *buf = dem->outBuf;
    float complex lastVal = dem->lastVal;
    while (size--)
        {
        float complex cpx = *data++;
        float complex prod = cpx * conj(lastVal);
        lastVal = cpx;
        float v = cabsf(prod);
        buf[bufPtr++] = v;
        if (bufPtr >= DEMOD_BUFSIZE)
            {
            int allocSize = bufPtr * sizeof(float);
            float *audiobuf = (float *)malloc(allocSize);
            memcpy(audiobuf, buf, allocSize);
            func(audiobuf, bufPtr, context); 
            bufPtr = 0;
            }
        }
    dem->lastVal = lastVal;
    dem->bufPtr  = bufPtr;
}


Demodulator *demodFmCreate()
{
    Demodulator *dem = (Demodulator *)smalloc(sizeof(Demodulator));
    if (!dem)
        return NULL;
    dem->bufPtr  = 0;
    dem->lastVal = 0;
    dem->update  = fmDemodulate;
    return dem;
}


void demodDelete(Demodulator *dem)
{
    free(dem);
}

