#ifndef _FFT_H_
#define _FFT_H_

/**
 * Fast fourier transform definitions and implementations.
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


typedef struct
{
} Fft;


/**
 * Create a new Fft instance.
 * @return a new Fft instance
 */  
Fft *fftCreate();


/**
 * Delete an Fft instance, stopping
 * any processing and freeing any resources.
 * @param fft an Fft instance.
 */   
void fftDelete(Fft *fft);



#endif /* _FFT_H_ */

