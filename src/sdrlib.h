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
    int x;
} SdrLib;

/**
 * Create a new SdrLib instance.
 * @return a new SdrLib instance
 */  
SdrLib *sdrCreate();

/**
 * Delete an SdrLib instance, stopping
 * any processing and freeing any resources.
 * @param sdrlib and SDRLib instance.
 */   
void sdrDelete(SdrLib *sdrlib);



#ifdef __cplusplus
}
#endif



#endif /* _SDRLIB_H_ */

