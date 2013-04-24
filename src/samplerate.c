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
    float omega = TWOPI * cutoffFreq / sampleRate;
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
        coeffs[idx] = (i == 0) ? (omega2-omega1) / PI : (sin(omega2*i) - sin(omega1 * i)) / (PI * i);
        }
}

//########################################################################
//#  D E C I M A T O R
//########################################################################

Decimator *decimatorCreate(int size, float highRate, float lowRate)
{
    Decimator *dec = (Decimator *)malloc(sizeof(Decimator));
    //FIR sizes must be odd
    size |= 1;
    dec->size = size;
    dec->coeffs = (float *)malloc(size * sizeof(float));
    decimatorSetRates(dec, highRate, lowRate);
    int delayLineSize = size * sizeof(float complex);
    dec->delayLine = (float complex *)malloc(delayLineSize);
    memset(dec->delayLine, 0, delayLineSize);
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

void decimatorUpdate(Decimator *dec, float complex *data, int dataLen, ComplexOutputFunc *func, void *context)
{
    int   size         = dec->size;
    int   size1        = size - 1;
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
                sum += delayLine[idx++] * (*coeff++);
                idx %= size;
                }
            //trace("sum:%f", sum * 1000.0);
            buf[bufPtr++] = sum;
            if (bufPtr >= DECIMATOR_BUFSIZE)
                {
                func(buf, bufPtr, context);
                bufPtr = 0;
                }
            }
        delayIndex = (delayIndex + size1) % size;
        }
    dec->delayIndex = delayIndex;
    dec->acc = acc;
    dec->bufPtr = bufPtr;
}



//########################################################################
//#  D D C
//########################################################################

DelayVal *delayCreate(int size)
{
    int allocSize = size * sizeof(DelayVal);
    DelayVal *xs = (DelayVal *)malloc(allocSize);
    if (!xs)
        return NULL;
    memset(xs, 0, allocSize);
    //initialization. slow and verbose, but clear
    int i = 0;
    for ( ; i < size ; i++)
        {
        if (i==0)
            {
            xs[i].prev = &(xs[size-1]);
            xs[i].next = &(xs[i+1]);
            }
        else if (i==size-1)
            {
            xs[i].prev = &(xs[i-1]);
            xs[i].next = xs;
            }
        else
            {
            xs[i].prev = &(xs[i-1]);
            xs[i].next = &(xs[i+1]);
            }
        }
    return xs;
}


void delayDelete(DelayVal *obj)
{
    if (obj)
        {
        free(obj);
        }
}



Ddc *ddcCreate(int size, float vfoFreq, float pbLoOff, float pbHiOff, float sampleRate)
{
    Ddc *obj = (Ddc *)malloc(sizeof(Ddc));
    //FIR sizes must be odd
    size |= 1;
    obj->size = size;
    obj->coeffs = (float *)malloc(size * sizeof(float));
    if (!obj->coeffs)
        {
        free(obj);
        return NULL;
        }
    obj->delayLine  = delayCreate(size);
    if (!obj->delayLine)
        {
        free(obj->coeffs);
        free(obj);
        return NULL;
        }
    obj->head = obj->delayLine;
    obj->inRate = sampleRate;
    ddcSetFreqs(obj, vfoFreq, pbLoOff, pbHiOff);
    obj->acc        = -1.0;
    obj->bufPtr     = 0;
    obj->vfoPhase   = 0.0 + 1.0 * I;
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
    float hiAbs = fabs(pbHiOff);
    float loAbs = fabs(pbLoOff);
    float maxOff = (hiAbs > loAbs) ? hiAbs : loAbs;
    firBPCoeffs(obj->size, obj->coeffs, pbLoOff, pbHiOff, obj->inRate);
    obj->outRate = maxOff * 2.0;
    obj->ratio = obj->outRate/obj->inRate;
    float omega = TWOPI * vfoFreq / obj->inRate;
    obj->vfoFreq = cos(omega) - sin(omega) * I;
}


float ddcGetOutRate(Ddc *obj)
{
    return obj->outRate;
}



/**
 * Downmix, downsample, and bandpass the input stream of sample, all in one go.
 *
 * For each data sample:
 *
 * 1.  Advance the VFO phase and convolve it with the sample
 * 2.  Increment the accumulator with the ratio until it is >= 0. then process a sample
 *     Example:  say the input rate is 1Ms/s and the desired output is 100ks/s.  Then the
 *     ratio is 0.1, and the decimation rate is 10.   The accumulator starts at -1. 
 *     We increment the accumulator 10 times with 0.1 until it reaches 0.0.  We reset the
 *     accumulator and process the sample.
 * 3.  Process the sample with the FIR bandbass coefficients,  with  the coefficients
 *     first to last, and the samples in the delay line in reverse going from most recent.
 * 4.  Add the sample to the output buffer
 * 5.  When the output buffer is full, call the output function, clear the buffer, and
 *     continue the loop.
 *
 * Re: the VFO
 * 
 * To advance the vfo's cosine and sine values, for mixing
 * with the incoming samples, look at the following trig
 * identities:
 *      
 * cos(a+b) = cos(a)*cos(b) - sin(a)*sin(b)
 * sin(a+b) = sin(a)*cos(b) + cos(a)*sin(b)
 * 
 * Let 'a' be the current phase angle, and let 'b' be
 * the increment added to that angle for every incoming
 * sample period.
 *  
 * Let omega be that angular increment (angular frequency) or 'b'
 *   
 *  float omega = 2.0 * Pi * freq / sampleRate  
 * 
 *  cosb = cos(omega)
 *  sinb = sin(omega)
 *  newcos = cos * cosb - sin * sinb
 *  newsin = sin * sinb + cos * sinb
 *  cos = newcos
 *  sin = newsin
 *  
 * Note that this cross-product is identical to a complex
 * multiplication, if the real part holds the cosine value,
 * and the imaginary part holds the sine.      
 * 
 * c.r =  a.r * b.r -  a.i * b.i
 * c.i =  a.r * b.i +  a.i * b.r
 * 
 * Let b be the phase and a be the angular frequency
 * newphase.r = phase.r * a.r - phase.i * a.i
 * newphase.i = phase.i * a.r + phase.i * a.i
 * 
 * so... newphase = phase * freq  
 *      
 * So, let
 *  float complex vfoFreq = cos(omega) + I * sin(omega);
 *  float complex vfoPhase = 0.0 + 1.0*I
 *   
 *  And for each sample,
 *     vfoPhase *= vfoFreq
 *     downshifted = sample * vfoPhase
 *     
 * So,  a lot of discussion, but extremely simple in the end!
 * 
 */     
void ddcUpdate(Ddc *obj, float complex *data, int dataLen, ComplexOutputFunc *func, void *context)
{
    int   size         = obj->size;
    int   size1        = size - 1;
    float *coeffs      = obj->coeffs;
    DelayVal *head     = obj->head;
    float ratio        = obj->ratio;
    float acc          = obj->acc;
    float complex *buf = obj->buf;
    int   bufPtr       = obj->bufPtr;
    
    while (dataLen--)
        {
        //advance the VFO and convolve the input stream
        obj->vfoPhase *= obj->vfoFreq;
        float complex sample = (*data++) * obj->vfoPhase;
        head->c = sample;
        //perform our fractional decimation
        //do the Bresenham's thing
        acc += ratio;
        if (acc > 0.0)
            {
            acc -= 1.0;
            float complex sum = 0.0;
            DelayVal *v = head;
            float *coeff = coeffs; 
            int c = size;
            while (c--)
                {
                sum += v->c * (*coeff++);
                v = v->prev;
                }
            //trace("sum:%f", sum * 1000.0);
            buf[bufPtr++] = sum;
            if (bufPtr >= DDC_BUFSIZE)
                {
                func(buf, DDC_BUFSIZE, context);
                bufPtr = 0;
                }
            }
        head = head->next;
        }
    obj->head = head;
    obj->acc  = acc;
    obj->bufPtr = bufPtr;
}


//########################################################################
//#  C I C
//########################################################################

#if 0

typedef struct Cic Cic;

struct Cic
{
    int ri, 
    long lastItgr[4];
    long comb[4];
    long lastComb[4]
    long last2Comb[4];
    float ratio;
    float acc;
};

/*
     integrator1 = integrator1 + xx ;
	 integrator2 = integrator2 + integrator1 ;
	 integrator3 = integrator3 + integrator2 ;
	 integrator4 = integrator4 + integrator3 ;
	 
	 downsample_clock++ ;
	 if ((downsample_clock & 0x03) == 0) //modulo 4
	 begin
		comb1 = integrator4 - last2_integrator4 ;
    	comb2 = comb1 - last2_comb1 ;
    	comb3 = comb2 - last2_comb2 ;
    	yy = (int)((comb3 - last2_comb3)>>16) ;
		// update state
    	last2_integrator4 = last_integrator4 ;
    	last_integrator4 = integrator4 ;
    	last2_comb1 = last_comb1 ;
    	last_comb1 = comb1;
    	last2_comb2 = last_comb2 ;
    	last_comb2 = comb2;
    	last2_comb3 = last_comb3 ;
    	last_comb3 = comb3;
*/

Cic *cicCreate(float inRate, float outRate)
{
    Cic *obj = (Cic *)malloc(sizeof (Cic));
    if (!obj)
        return NULL;
    memset(obj, 0, sizeof(Cic));
    obj->ratio = outRate - inRate;
    return obj;
}

void cicDelete(Cic *obj)
{
    if (obj)
        {
        free(obj);
        }
}


void cicUpdate(Cic *obj, float complex *data, int datalen, ComplexFunc func, void *ctx)
{
    float ratio = obj->ratio;
    float acc = obj->acc;
    while (datalen--)
        {
        int rvalu
        
        }
        
    obj->acc = acc;
    obj->bufptr = bufPtr;
}





#endif

//########################################################################
//#  R E S A M P L E R
//########################################################################



Resampler *resamplerCreate(int size, float inRate, float outRate)
{
    Resampler *obj = (Resampler *)malloc(sizeof(Resampler));
    if (!obj)
        return NULL;
    //FIR sizes must be odd
    size |= 1;
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
    firLPCoeffs(obj->size, obj->coeffs, outRate, inRate);
    obj->updown = (outRate > inRate);
    obj->ratio  = (obj->updown) ? inRate/outRate : outRate/inRate;
    trace("in:%f out:%f ud:%d, ratio:%f", inRate, outRate, obj->updown, obj->ratio);
}

void resamplerSetOutRate(Resampler *obj, float outRate)
{
    float inRate = obj->inRate;
    obj->outRate = outRate;
    firLPCoeffs(obj->size, obj->coeffs, outRate, inRate);
    obj->updown = (outRate > inRate);
    obj->ratio  = (obj->updown) ? inRate/outRate : outRate/inRate;
}




void resamplerUpdate(Resampler *obj, float *data, int dataLen, FloatOutputFunc *func, void *context)
{
    int   size       = obj->size;
    int   size1      = size-1;
    float *coeffs    = obj->coeffs;
    float *delayLine = obj->delayLine;
    int   delayIndex = obj->delayIndex;
    float ratio      = obj->ratio;
    float acc        = obj->acc;
    float *buf       = obj->buf;
    int   bufPtr     = obj->bufPtr;
    
    if (obj->updown)
        {
        //interpolate
        while (dataLen--)
            {
            delayLine[delayIndex] = *data++;
            acc -= 1.0;
            while (acc < 0.0)
                {
                acc += ratio;
                float sum = 0.0;
                int idx = delayIndex;
                float *coeff = coeffs; 
                int c = size;
                while (c--)
                    {
                    sum += delayLine[idx++] * (*coeff++);
                    idx %= size;
                    }
                buf[bufPtr++] = sum;
                if (bufPtr >= RESAMPLER_BUFSIZE)
                    {
                    (*func)(buf, RESAMPLER_BUFSIZE, context);
                    bufPtr = 0;
                    }
                }
            delayIndex = (delayIndex + size1) % size;
            }
        }
    else
        {
        //decimate
        while (dataLen--)
            {
            delayLine[delayIndex] = *data++;
            acc += ratio;
            if (acc >= 0.0)
                {
                acc -= 1.0;
                float sum = 0.0;
                int idx = delayIndex;
                float *coeff = coeffs; 
                int c = size;
                while (c--)
                    {
                    sum += delayLine[idx++] * (*coeff++);
                    idx %= size;
                    }
                buf[bufPtr++] = sum;
                if (bufPtr >= RESAMPLER_BUFSIZE)
                    {
                    (*func)(buf, RESAMPLER_BUFSIZE, context);
                    bufPtr = 0;
                    }
                }
            delayIndex = (delayIndex + size1) % size;
            }
        }
    obj->delayIndex = delayIndex;
    obj->acc = acc;
    obj->bufPtr = bufPtr;
}


void resamplerUpdateC(Resampler *obj, float complex *data, int dataLen, ComplexOutputFunc *func, void *context)
{
    int   size         = obj->size;
    int   size1        = size-1;
    float *coeffs      = obj->coeffs;
    float complex *delayLine = obj->delayLineC;
    int   delayIndex   = obj->delayIndex;
    float ratio        = obj->ratio;
    float acc          = obj->acc;
    float complex *buf = obj->bufC;
    int   bufPtr       = obj->bufPtr;
    
    if (obj->updown)
        {
        //interpolate
        while (dataLen--)
            {
            delayLine[delayIndex] = *data++;
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
                    sum += delayLine[idx++] * (*coeff++);
                    idx %= size;
                    }
                buf[bufPtr++] = sum;
                if (bufPtr >= RESAMPLER_BUFSIZE)
                    {
                    (*func)(buf, RESAMPLER_BUFSIZE, context);
                    bufPtr = 0;
                    }
                }
            delayIndex = (delayIndex + size1) % size;
            }
        }
    else
        {
        //decimate
        while (dataLen--)
            {
            delayLine[delayIndex] = *data++;
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
                    sum += delayLine[idx++] * (*coeff++);
                    idx %= size;
                    }
                buf[bufPtr++] = sum;
                if (bufPtr >= RESAMPLER_BUFSIZE)
                    {
                    (*func)(buf, RESAMPLER_BUFSIZE, context);
                    bufPtr = 0;
                    }
                }
            delayIndex = (delayIndex + size1) % size;
            }
        }
    obj->delayIndex = delayIndex;
    obj->acc = acc;
    obj->bufPtr = bufPtr;
}


