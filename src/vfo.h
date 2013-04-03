#ifndef _VFO_H_
#define _VFO_H_

/**
 * VFO
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
 
#include "sdrlib.h"

#include <complex.h>

/**
 *
 */
struct Vfo
{
    float sampleRate;
    float complex phase;
    float complex freq;
};


/**
 *
 */
Vfo *vfoCreate(float frequency, float sampleRate);

/**
 *
 */
void vfoDelete(Vfo *vfo);

/**
 *
 */
void vfoSetFrequency(Vfo *vfo, float frequency);

/**
 *
 */
float complex vfoUpdate(Vfo *vfo, float complex sample);



#endif /* _VFO_H_ */

