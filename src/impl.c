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

#include <string.h>
#include <stdlib.h>
#include "impl.h"
#include "audio.h"
#include "demod.h"
#include "device.h"
#include "filter.h"
#include "private.h"
#include "vfo.h"


static void *implReaderThread(void *ctx);


/**
 */  
int implCreate(SdrLib *lib)
{
    Impl * impl = (Impl *) malloc(sizeof(Impl));
    memset(impl, 0, sizeof(Impl));
    lib->impl = impl;
    impl->deviceCount = deviceScan(DEVICE_SDR, impl->devices, MAX_DEVICES);
    if (!impl->deviceCount)
        {
        error("No devices found");
        //but dont fail. wait until start()
        }
    impl->fft = fftCreate(16384);
    impl->vfo = vfoCreate(0.0, 2048000.0);
    impl->bpf = firBP(11, -50000.0, 50000.0, 2048000.0, W_HAMMING);
    impl->decimator = decimatorCreate(11, 2048000.0, 44100.0);
    impl->demodFm = demodFmCreate();
    impl->demodAm = demodAmCreate();
    impl->demod = impl->demodFm; //TODO: make user-settable
    impl->audio = audioCreate();
    return TRUE;
}



/**
 */   
int implClose(SdrLib *lib)
{
    Impl *impl = (Impl *)lib->impl;
    int i=0;
    for (; i < impl->deviceCount ; i++)
        {
        Device *d = impl->devices[i];
        d->delete(d->ctx);
        }
    fftDelete(impl->fft);
    vfoDelete(impl->vfo);
    firDelete(impl->bpf);
    decimatorDelete(impl->decimator);
    demodDelete(impl->demodFm);
    demodDelete(impl->demodAm);
    free(impl);
    lib->impl = NULL;
    return TRUE;
}




/**
 */   
int implStart(SdrLib *lib)
{
    Impl *impl = (Impl *)lib->impl;
    pthread_t thread;
    if (!impl->deviceCount)
        {
        error("No devices found");
        return FALSE;
        }
    Device *d = impl->devices[0];
    if (!d->open(d->ctx))
        {
        error("Could not start device");
        return FALSE;
        }
    impl->device = d;
    d->setGain(d->ctx, 1.0);
    d->setCenterFrequency(d->ctx, 88700000.0);
    impl->keepGoing = 1;
    int rc = pthread_create(&thread, NULL, implReaderThread, (void *)impl);
    if (rc)
        {
        error("ERROR; return code from pthread_create() is %d", rc);
        return FALSE;
        }
    return TRUE;
}


/**
 */   
int implStop(SdrLib *lib)
{
    Impl *impl = (Impl *)lib->impl;
    impl->device->close(impl->device->ctx);
    impl->keepGoing = 0;
    void *status;
    pthread_join(impl->thread, &status);
    return TRUE;
}


#define READSIZE (8 * 16384)

static void fftOutput(unsigned int *vals, int size, void *ctx)
{
    Impl *impl = (Impl *)ctx;
    trace("got power spectrum size:%d", size);
}

static void demodOutput(float *buf, int size, void *ctx)
{
    Impl *impl = (Impl *)ctx;
    trace("Push audio:%d", size);
    audioPlay(impl->audio, buf, size);
}

static void decimatorOutput(float complex *data, int size, void *ctx)
{
    Impl *impl = (Impl *)ctx;
    impl->demod->update(impl->demod, data, size, demodOutput, impl);
}

static void *implReaderThread(void *ctx)
{
    Impl *impl = (Impl *)ctx;
    Device *dev = impl->device;
    
    float complex *readbuf = (float complex *) malloc(READSIZE * sizeof(float complex));
    
    while (impl->keepGoing)
        {
        int count = dev->read(dev->ctx, readbuf, READSIZE);
        trace("read: %d", count);
        fftUpdate(impl->fft, readbuf, count, fftOutput, impl);
        decimatorUpdate(impl->decimator, readbuf, count, decimatorOutput, impl);
        }

    free(readbuf);
    return NULL;
}
