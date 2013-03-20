#ifndef _DEVICE_H_
#define _DEVICE_H_

/**
 * API for adding devices to this lib
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
#include "util.h"

typedef enum
{
    DEVICE_NONE,
    DEVICE_SDR
} DeviceType;




typedef struct Device Device;



/**
 * This API for a Device tries to provide a uniform
 * model for controlling different devices.  It is
 * up to writers of the "glue" for each device to
 * provide the mapping to/from the real devices.
 */
struct Device
{
    int    type;
    char   *name;
    
    /**
     * Pointer to context information provided by the device.  This will be passed
     * back in subsequent calls to the functions.
     */
    void *ctx;
    
    /**
     * All devices must provide a function to shut down and free resources,
     * including the context and struct.
     */
    void (*close)(void *ctx);
    
    
    /**
     * Set the gain in the range of 0.0 - 1.0
     * @return true if successful, else false
     */
    int (*setGain)(void *ctx, float gain);
    /**
     * Get the gain in the range of 0.0 - 1.0
     */
    float(*getGain)(void *ctx);
    
    /**
     * Set the sample rate in samples per second
     * @return true if successful, else false
     */
    int (*setSampleRate)(void *ctx, float rate);
    /**
     * Get the sample rate in samples per second
     */
    float (*getSampleRate)(void *ctx);
    
    
    /**
     * Set the center frequency of the SDR device
     * @return true if successful, else false
     */
    int (*setCenterFrequency)(void *ctx, double freq);
    /**
     * Get the center frequency of the SDR device
     */
    double (*getCenterFrequency)(void *ctx);
    
    /**
     * Read/write complex data to/from the device
     * @return true if successful, else false
     */
    int (*read)(void *ctx, float complex *buf, int bufferlen);
    /**
     * Read/write complex data to/from the device
     * @return true if successful, else false
     */
    int (*write)(void *ctx, float complex *buf, int datalen);
    
    /**
     * True to set the device into transmit mode
     * @return true if successful, else false
     */
    int (*transmit)(void *ctx, int truefalse);
};


List *deviceScan(int type);


typedef struct Parent Parent;

struct Parent
{
    void (*trace)(char *format, ...);
    void (*error)(char *format, ...);
};


typedef int DeviceOpenFunc(Device *, Parent *);



#endif /* _DEVICE_H_ */


