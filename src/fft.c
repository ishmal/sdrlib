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
#include <stdlib.h>
#include <sdrlib.h>

#include "fft.h"

/**
 * Create a new Fft instance.
 * @return a new Fft instance
 */  
Fft *fftCreate()
{
    Fft *inst = (Fft *)malloc(sizeof(Fft));
    if (!inst)
        return inst;
    //TODO: setup here
    return inst;
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
    //TODO: clean up here
    free(fft);
}

