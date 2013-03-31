#ifndef _SDRLIB_H_
#define _SDRLIB_H_
/**
 * This is intended as a small and simple
 * SDR library with as low code complexity, 
 * and as few dependencies as possible.  
 * 
 * The goal is to make code readability, maintainability
 * and portability as high as possible.   
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



#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    void *impl;
} SdrLib;

/**
 * Create a new SdrLib instance.
 * @return a new SdrLib instance
 */  
SdrLib *sdrCreate();


/**
 * Delete an SdrLib instance, stopping
 * any processing and freeing any resources.
 * @param sdrlib an SDRLib instance.
 */   
int sdrDelete(SdrLib *sdrlib);



/**
 * Start sdrlib processing
 * @param sdrlib an SDRLib instance.
 */   
int sdrStart(SdrLib *sdrlib);



/**
 * Stop sdrlib processing
 * @param sdrlib an SDRLib instance.
 */   
int sdrStop(SdrLib *sdrlib);

/**
 * Get the current center frequency
 * @param sdrlib an SDRLib instance.
 */   
double sdrGetCenterFrequency(SdrLib *sdrlib);



/**
 * Stop sdrlib processing
 * @param sdrlib an SDRLib instance.
 */   
int sdrSetCenterFrequency(SdrLib *sdrlib, double freq);

/**
 * Get gain 0-1
 * @param sdrlib an SDRLib instance.
 */   
float sdrGetRfGain(SdrLib *sdrlib);



/**
 * Set gain 0-1
 * @param sdrlib an SDRLib instance.
 */   
int sdrSetRfGain(SdrLib *sdrlib, float gain);


/**
 * Get gain 0-1
 * @param sdrlib an SDRLib instance.
 */   
float sdrGetAfGain(SdrLib *sdrlib);



/**
 * Set gain 0-1
 * @param sdrlib an SDRLib instance.
 */   
int sdrSetAfGain(SdrLib *sdrlib, float gain);


typedef void PowerSpectrumFunc(unsigned int *ps, int size, void *ctx);

void sdrSetPowerSpectrumFunc(SdrLib *sdrlib, PowerSpectrumFunc *func, void *ctx);


#ifdef __cplusplus
}
#endif



#endif /* _SDRLIB_H_ */

