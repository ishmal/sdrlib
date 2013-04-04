#ifndef _FILTER_H_
#define _FILTER_H_
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

#include <complex.h>


#include "sdrlib.h"


//########################################################################
//#  F I R    F I L T E R S
//########################################################################


/**
 * Defines a base for a FIR filter
 */

struct Fir
{
    int size;
    float *coeffs;
    int delayIndex;
    float *delayLine;
    float complex *delayLineC;
};


enum
{
    W_NONE,
    W_HAMMING,
    W_HANN,
    W_BLACKMAN
} WindowType;



/**
 * Creates an empty FIR filter with no coefficients.
 * Users would normally not use this, rather use a factory function.
 */
Fir *firCreate(int size);

/**
 * Frees up a FIR filter and any allocated resources
 */
void firDelete(Fir *fir);


/**
 * Update a real-valued FIR filter with a sample.
 * @param fir the filter to update
 * @param sample the sample to add
 * @return the current output of the filter
 */
float firUpdate(Fir *fir, float sample);



/**
 * Update a complex-valued FIR filter with a sample.
 * @param fir the filter to update
 * @param sample the sample to add
 * @return the current output of the filter
 */
float complex firUpdateC(Fir *fir, float complex sample);


/**
 * Create a FIR lowpass filter
 */
Fir *firLP(int size, float cutoffFreq, float sampleRate, int windowType);


/**
 * Create a FIR highpass filter
 */
Fir *firHP(int size, float cutoffFreq, float sampleRate, int windowType);


/**
 * Create a FIR bandpass filter
 */
Fir *firBP(int size, float loCutoffFreq, float hiCutoffFreq, float sampleRate, int windowType);



//########################################################################
//#  B I Q U A D
//########################################################################

struct Biquad
{
    float b0;
    float b1;
    float b2;
    float a0;
    float a1;
    float a2;
    float x1;
    float x2;
    float y1;
    float y2;
    float complex x1c;
    float complex x2c;
    float complex y1c;
    float complex y2c;
};



void biquadDelete(Biquad *bq);

float biquadUpdate(Biquad *bq, float v);

float complex biquadUpdateC(Biquad *bq, float complex v);

Biquad *biquadLP(float frequency, float sampleRate, float q);

Biquad *biquadHP(float frequency, float sampleRate, float q);

Biquad *biquadBP(float frequency, float sampleRate, float q);

Biquad *biquadBR(float frequency, float sampleRate, float q);






#endif /* _FILTER_H_ */


