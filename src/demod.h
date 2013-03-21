#ifndef _DEMOD_H_
#define _DEMOD_H_

/**
 * Various demodulation types
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


typedef void DemodOutputFunc(float *buf, int size, void *context);

#define DEMOD_BUFSIZE 40000

typedef struct Demodulator Demodulator;

struct Demodulator
{
    float outBuf[DEMOD_BUFSIZE];
    int   bufPtr;
    float complex lastVal;
    void (*update)(Demodulator *dem, float complex *data, int size, DemodOutputFunc *func, void *context);
};



Demodulator *demodAmCreate();
Demodulator *demodFmCreate();
void demodDelete(Demodulator *dem);


#endif /* _DEMOD_H_ */

