#ifndef _IMPL_H_
#define _IMPL_H_

/**
 * This is the core engine of the sdr library  
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



#include <pthread.h>
#include "sdrlib.h"
#include "audio.h"
#include "demod.h"
#include "device.h"
#include "filter.h"
#include "fft.h"
#include "vfo.h"

typedef struct Impl Impl;

#define MAX_DEVICES 30

struct Impl
{
    int deviceCount;
    Device *devices[MAX_DEVICES];
    Device *device;
    pthread_t thread;
    int running; //state of the reader thread
    Fft *fft;
    PowerSpectrumFunc *psFunc;
    void *psFuncCtx; 
    Vfo *vfo;
    Fir *bpf;
    Decimator *decimator;
    Demodulator *demod;
    Demodulator *demodAm;
    Demodulator *demodFm;
    Audio *audio;
};


/**
 */  
int implCreate(SdrLib *sdr);


/**
 */   
int implDelete(Impl *impl);



/**
 */   
int implStart(Impl *impl);


/**
 */   
int implStop(Impl *impl);


/**
 */   
double implGetCenterFrequency(Impl *impl);


/**
 */   
int implSetCenterFrequency(Impl *impl, double freq);


/**
 */   
float implGetGain(Impl *impl);


/**
 */   
int implSetGain(Impl *impl, float gain);



#endif /* _IMPL_H_ */

