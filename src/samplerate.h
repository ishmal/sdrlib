#ifndef _SAMPLERATE_H_
#define _SAMPLERATE_H_
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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <complex.h>


#include "sdrlib.h"



//########################################################################
//#  D E C I M A T O R 
//#  A special type of FIR
//########################################################################


#define DECIMATOR_BUFSIZE (16384)

struct Decimator
{
    int size;
    float *coeffs;
    float complex *delayLine;
    int delayIndex;
    float ratio;
    float acc;
    float complex buf[DECIMATOR_BUFSIZE];
    int bufPtr;
};

Decimator *decimatorCreate(int size, float highRate, float lowRate);

void decimatorDelete(Decimator *dec);

void decimatorSetRates(Decimator *dec, float highRate, float lowRate);

typedef void DecimatorFunc(float complex *data, int size, void *context);

void decimatorUpdate(Decimator *dec, float complex *data, int dataLen, DecimatorFunc *func, void *context);


//########################################################################
//#  D D C
//########################################################################


#define DDC_BUFSIZE (16384)


struct Ddc
{
    int   size;
    float *coeffs;
    float complex *delayLine;
    int   delayIndex;
    float ratio;
    float inRate;
    float outRate;
    float complex phase;
    float complex freq;
    float acc;
    float complex buf[DECIMATOR_BUFSIZE];
    int   bufPtr;
};

Ddc *ddcCreate(int size, float vfoFreq, float pbLoOff, float pbHiOff, float sampleRate);

void ddcDelete(Ddc *obj);

void ddcSetFreqs(Ddc *obj, float vfoFreq, float pbLoOff, float pbHiOff);

typedef void DdcFunc(float complex *data, int size, void *context);

void ddcUpdate(Ddc *obj, float complex *data, int dataLen, DdcFunc *func, void *context);


//########################################################################
//#  R E S A M P L E R
//#  A more general version of the decimator.  Goes both directions
//########################################################################


#define RESAMPLER_BUFSIZE (16384)



struct Resampler
{
    int size;
    float *coeffs;
    float *delayLine;
    float complex *delayLineC;
    int delayIndex;
    float ratio;
    int updown;
    float acc;
    float buf[RESAMPLER_BUFSIZE];
    float complex bufC[RESAMPLER_BUFSIZE];
    int bufPtr;
};

Resampler *resamplerCreate(int size, float highRate, float lowRate);

void resamplerDelete(Resampler *obj);

typedef void ResamplerFunc(float *data, int size, void *context);
typedef void ResamplerFuncC(float complex *data, int size, void *context);

void resamplerUpdate(Resampler *obj, float *data, int dataLen, ResamplerFunc *func, void *context);
void resamplerUpdateC(Resampler *obj, float complex *data, int dataLen, ResamplerFuncC *func, void *context);




#endif /* _SAMPLERATE_H_ */


