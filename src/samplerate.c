/**
 * Decimation, interpolation, and DDC  
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
 *  but WITHOUT ANY WARRANTY; without even the sdried warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "samplerate.h"
#include "private.h"

/**
 * Copied from filter.c.   There is probably a better way
 */
static void firLPCoeffs(int size, float *coeffs, float cutoffFreq, float sampleRate)
{
    float omega = 2.0 * PI * cutoffFreq / sampleRate;
    int center = (size - 1) / 2;
    int idx = 0;
    for ( ; idx < size ; idx++)
        {
        int i = idx - center;
        coeffs[idx] = (i == 0) ? omega / PI : sin(omega * i) / (PI * i);
        }
}

static void firBPCoeffs(int size, float *coeffs, float loCutoffFreq, float hiCutoffFreq, float sampleRate)
{
    float omega1 = TWOPI * loCutoffFreq / sampleRate;
    float omega2 = TWOPI * hiCutoffFreq / sampleRate;
    int center = (size - 1) / 2;
    int idx = 0;
    for ( ; idx < size ; idx++)
        {
        int i = idx - center;
        coeffs[idx] = (i == 0) ? 
            1.0 - (omega2-omega1) / PI :
            (sin(omega1*i) - sin(omega2 * i)) / (PI * i);
        }
}

//########################################################################
//#  D E C I M A T O R
//########################################################################

Decimator *decimatorCreate(int size, float highRate, float lowRate)
{
    Decimator *dec = (Decimator *)malloc(sizeof(Decimator));
    dec->size = size;
    dec->coeffs = (float *)malloc(size * sizeof(float));
    decimatorSetRates(dec, highRate, lowRate);
    dec->delayLine = (float complex *)malloc(size * sizeof(float complex));
    int i = 0;
    for (; i < size ; i++)
        dec->delayLine[i] = 0;
    dec->delayIndex = 0;
    dec->acc = 0.0;
    dec->bufPtr = 0;
    return dec;
}


void decimatorSetRates(Decimator *dec, float highRate, float lowRate)
{
    firLPCoeffs(dec->size, dec->coeffs, lowRate, highRate);
    dec->ratio = lowRate/highRate;
}


void decimatorDelete(Decimator *dec)
{
    if (dec)
        {
        free(dec->delayLine);
        free(dec->coeffs);
        free(dec);
        }
}

void decimatorUpdate(Decimator *dec, float complex *data, int dataLen, DecimatorFunc *func, void *context)
{
    int   size         = dec->size;
    float *coeffs      = dec->coeffs;
    float complex *delayLine = dec->delayLine;
    int   delayIndex   = dec->delayIndex;
    float ratio        = dec->ratio;
    float acc          = dec->acc;
    float complex *buf = dec->buf;
    int   bufPtr       = dec->bufPtr;
    
    float complex *cpx = data;
    while (dataLen--)
        {
        delayLine[delayIndex] = *cpx++;
        acc += ratio;
        if (acc > 0.0)
            {
            acc -= 1.0;
            float complex sum = 0.0;
            //walk the coefficients from first to last
            //and the delay line from newest to oldest
            int idx = delayIndex;
            float *coeff = coeffs; 
            int c = size;
            while (c--)
                {
                float complex v = delayLine[idx];
                idx -= 1;
                if (idx < 0)
                    idx = size - 1;
                sum += v * (*coeff++);
                }
            //trace("sum:%f", sum * 1000.0);
            buf[bufPtr++] = sum;
            if (bufPtr >= DECIMATOR_BUFSIZE)
                {
                func(buf, bufPtr, context);
                bufPtr = 0;
                }
            }
        delayIndex = (delayIndex + 1) % size;
        }
    dec->delayIndex = delayIndex;
    dec->acc = acc;
    dec->bufPtr = bufPtr;
}



//########################################################################
//#  D D C
//########################################################################

Ddc *ddcCreate(int size, float vfoFreq, float pbLoOff, float pbHiOff, float sampleRate)
{
    Ddc *obj = (Ddc *)malloc(sizeof(Ddc));
    obj->size = size;
    obj->coeffs = (float *)malloc(size * sizeof(float));
    obj->inRate = sampleRate;
    ddcSetFreqs(obj, vfoFreq, pbLoOff, pbHiOff);
    int delayLineSize = size * sizeof(float complex);
    obj->delayLine = (float complex *)malloc(delayLineSize);
    memset(obj->delayLine, 0, delayLineSize);
    obj->delayIndex = 0;
    obj->acc = 0.0;
    obj->bufPtr = 0;
    obj->phase = 0.0 + I * 1.0;
    return obj;
}

void ddcDelete(Ddc *obj)
{
    if (obj)
        {
        free(obj->delayLine);
        free(obj->coeffs);
        free(obj);
        }
}

void ddcSetFreqs(Ddc *obj, float vfoFreq, float pbLoOff, float pbHiOff)
{
    float loFreq = vfoFreq+pbLoOff;
    float hiFreq = vfoFreq+pbHiOff;
    firBPCoeffs(obj->size, obj->coeffs, loFreq, hiFreq, obj->inRate);
    obj->outRate = (hiFreq - loFreq) * 1.5;
    obj->ratio = obj->outRate/obj->inRate;
    float angle = TWOPI * vfoFreq / obj->inRate;
    obj->freq = cos(angle) + I * sin(angle);
}


float ddcGetOutRate(Ddc *obj)
{
    return obj->outRate;
}

void ddcUpdate(Ddc *obj, float complex *data, int dataLen, DdcFunc *func, void *context)
{
    int   size         = obj->size;
    float *coeffs      = obj->coeffs;
    float complex *delayLine = obj->delayLine;
    int   delayIndex   = obj->delayIndex;
    float ratio        = obj->ratio;
    float acc          = obj->acc;
    float complex *buf = obj->buf;
    int   bufPtr       = obj->bufPtr;
    
    float complex *cpx = data;
    while (dataLen--)
        {
        obj->phase *= obj->freq;
        delayLine[delayIndex] = (*cpx++) * obj->phase;
        acc += ratio;
        if (acc > 0.0)
            {
            acc -= 1.0;
            float complex sum = 0.0;
            //walk the coefficients from first to last
            //and the delay line from newest to oldest
            int idx = delayIndex;
            float *coeff = coeffs; 
            int c = size;
            while (c--)
                {
                float complex v = delayLine[idx];
                idx -= 1;
                if (idx < 0)
                    idx = size - 1;
                //trace("coeff:%f",*coeff);
                sum += v * (*coeff++);
                }
            //trace("sum:%f", sum * 1000.0);
            buf[bufPtr++] = sum;
            if (bufPtr >= DECIMATOR_BUFSIZE)
                {
                func(buf, bufPtr, context);
                bufPtr = 0;
                }
            }
        delayIndex = (delayIndex + 1) % size;
        }
    obj->delayIndex = delayIndex;
    obj->acc = acc;
    obj->bufPtr = bufPtr;
}


//########################################################################
//#  R E S A M P L E R
//########################################################################



Resampler *resamplerCreate(int size, float inRate, float outRate)
{
    Resampler *obj = (Resampler *)malloc(sizeof(Resampler));
    obj->size = size;
    obj->coeffs = (float *)malloc(size * sizeof(float));
    firLPCoeffs(size, obj->coeffs, outRate, inRate);
    obj->delayLine  = (float *)malloc(size * sizeof(float));
    obj->delayLineC = (float complex *)malloc(size * sizeof(float complex));
    int i = 0;
    for (; i < size ; i++)
        {
        obj->delayLine[i] = 0;
        obj->delayLineC[i] = 0;
        }
    obj->delayIndex = 0;
    obj->inRate = inRate;
    obj->outRate = outRate;
    obj->updown = (outRate > inRate);
    obj->ratio = (obj->updown) ? inRate/outRate : outRate/inRate;
    obj->acc = 0.0;
    obj->bufPtr = 0;
    return obj;
}


void resamplerDelete(Resampler *obj)
{
    if (obj)
        {
        free(obj->delayLine);
        free(obj->delayLineC);
        free(obj->coeffs);
        free(obj);
        }
}


void resamplerSetInRate(Resampler *obj, float inRate)
{
    float outRate = obj->outRate;
    obj->inRate = inRate;
    obj->updown = (outRate > inRate);
    obj->ratio  = (obj->updown) ? inRate/outRate : outRate/inRate;
}

void resamplerSetOutRate(Resampler *obj, float outRate)
{
    float inRate = obj->inRate;
    obj->outRate = outRate;
    obj->updown = (outRate > inRate);
    obj->ratio  = (obj->updown) ? inRate/outRate : outRate/inRate;
}


void resamplerUpdate(Resampler *obj, float *data, int dataLen, ResamplerFunc *func, void *context)
{
    int   size         = obj->size;
    float *coeffs      = obj->coeffs;
    float *delayLine   = obj->delayLine;
    int   delayIndex   = obj->delayIndex;
    float ratio        = obj->ratio;
    float acc          = obj->acc;
    float *buf         = obj->buf;
    int   bufPtr       = obj->bufPtr;
    
    float *sample = data;
    
    if (obj->updown)
        {
        //interpolate
        while (dataLen--)
            {
            delayLine[delayIndex] = *sample++;
            acc -= 1.0;
            while (acc < 0.0)
                {
                acc += ratio;
                float complex sum = 0.0;
                int idx = delayIndex;
                float *coeff = coeffs; 
                int c = size;
                while (c--)
                    {
                    float complex v = delayLine[idx];
                    idx -= 1;
                    if (idx < 0)
                        idx = size - 1;
                    sum += v * (*coeff++);
                    }
                buf[bufPtr++] = sum;
                if (bufPtr >= DECIMATOR_BUFSIZE)
                    {
                    func(buf, bufPtr, context);
                    bufPtr = 0;
                    }
                }
            delayIndex = (delayIndex + 1) % size;
            }
        }
    else
        {
        //decimate
        while (dataLen--)
            {
            delayLine[delayIndex] = *sample++;
            acc += ratio;
            if (acc > 0.0)
                {
                acc -= 1.0;
                float complex sum = 0.0;
                int idx = delayIndex;
                float *coeff = coeffs; 
                int c = size;
                while (c--)
                    {
                    float complex v = delayLine[idx];
                    idx -= 1;
                    if (idx < 0)
                        idx = size - 1;
                    sum += v * (*coeff++);
                    }
                buf[bufPtr++] = sum;
                if (bufPtr >= DECIMATOR_BUFSIZE)
                    {
                    func(buf, bufPtr, context);
                    bufPtr = 0;
                    }
                }
            delayIndex = (delayIndex + 1) % size;
            }
        }
    obj->delayIndex = delayIndex;
    obj->acc = acc;
    obj->bufPtr = bufPtr;
}


void resamplerUpdateC(Resampler *obj, float complex *data, int dataLen, ResamplerFuncC *func, void *context)
{
    int   size         = obj->size;
    float *coeffs      = obj->coeffs;
    float complex *delayLine = obj->delayLineC;
    int   delayIndex   = obj->delayIndex;
    float ratio        = obj->ratio;
    float acc          = obj->acc;
    float complex *buf = obj->bufC;
    int   bufPtr       = obj->bufPtr;
    
    float complex *sample = data;
    
    if (obj->updown)
        {
        //interpolate
        while (dataLen--)
            {
            delayLine[delayIndex] = *sample++;
            acc -= 1.0;
            while (acc < 0.0)
                {
                acc += ratio;
                float complex sum = 0.0;
                int idx = delayIndex;
                float *coeff = coeffs; 
                int c = size;
                while (c--)
                    {
                    float complex v = delayLine[idx];
                    idx -= 1;
                    if (idx < 0)
                        idx = size - 1;
                    sum += v * (*coeff++);
                    }
                buf[bufPtr++] = sum;
                if (bufPtr >= DECIMATOR_BUFSIZE)
                    {
                    func(buf, bufPtr, context);
                    bufPtr = 0;
                    }
                }
            delayIndex = (delayIndex + 1) % size;
            }
        }
    else
        {
        //decimate
        while (dataLen--)
            {
            delayLine[delayIndex] = *sample++;
            acc += ratio;
            if (acc > 0.0)
                {
                acc -= 1.0;
                float complex sum = 0.0;
                int idx = delayIndex;
                float *coeff = coeffs; 
                int c = size;
                while (c--)
                    {
                    float complex v = delayLine[idx];
                    idx -= 1;
                    if (idx < 0)
                        idx = size - 1;
                    sum += v * (*coeff++);
                    }
                buf[bufPtr++] = sum;
                if (bufPtr >= DECIMATOR_BUFSIZE)
                    {
                    func(buf, bufPtr, context);
                    bufPtr = 0;
                    }
                }
            delayIndex = (delayIndex + 1) % size;
            }
        }
    obj->delayIndex = delayIndex;
    obj->acc = acc;
    obj->bufPtr = bufPtr;
}

