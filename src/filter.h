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

/**
 * Defines a base for a FIR filter
 */

typedef struct
{
    int size;
    float *coeffs;
    int delayIndex;
    float *delayLine;
} Fir;

typedef struct
{
    int size;
    float *coeffs;
    int delayIndex;
    complex *delayLine;
} FirC;

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
 * Creates an empty FIR filter with no coefficients.
 * Users would normally not use this, rather use a factory function.
 */
FirC *firCreateC(int size);

/**
 * Frees up a FIR filter and any allocated resources
 */
void firDeleteC(FirC *fir);


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
complex firUpdateC(FirC *fir, complex sample);


/**
 * Create a FIR lowpass filter
 */
Fir *firLP(int size, float cutoffFreq, float sampleRate, int windowType);


/**
 * Create a FIR lowpass filter
 */
FirC *firLPC(int size, float cutoffFreq, float sampleRate, int windowType);


#endif /* _FILTER_H_ */


