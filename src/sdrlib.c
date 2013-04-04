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
 *  but WITHOUT ANY WARRANTY; without even the sdried warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string.h>
#include <stdlib.h>


#include "sdrlib.h"

#include "audio.h"
#include "demod.h"
#include "device.h"
#include "fft.h"
#include "filter.h"
#include "samplerate.h"
#include "vfo.h"

#include "private.h"


static void *sdrReaderThread(void *ctx);


/**
 */  
SdrLib *sdrCreate()
{
    SdrLib * sdr = (SdrLib *) malloc(sizeof(SdrLib));
    memset(sdr, 0, sizeof(SdrLib));
    sdr->deviceCount = deviceScan(DEVICE_SDR, sdr->devices, SDR_MAX_DEVICES);
    if (!sdr->deviceCount)
        {
        error("No devices found");
        //but dont fail. wait until start()
        }
    sdr->fft = fftCreate(16384);
    //sdr->vfo = vfoCreate(0.0, 2048000.0);
    //sdr->bpf = firBP(11, -50000.0, 50000.0, 2048000.0, W_HAMMING);
    sdr->ddc = ddcCreate(11, 0.0, -5000.0, 5000.0, 2048000.0);
    sdr->demodFm = demodFmCreate();
    sdr->demodAm = demodAmCreate();
    sdr->demod = sdr->demodFm;
    sdr->resampler  = resamplerCreate(11, 44100.0, 44100.0);
    sdr->audio = audioCreate();
    return sdr;
}



/**
 */   
int sdrDelete(SdrLib *sdr)
{
    int i=0;
    for (; i < sdr->deviceCount ; i++)
        {
        Device *d = sdr->devices[i];
        d->delete(d->ctx);
        }
    audioDelete(sdr->audio);
    fftDelete(sdr->fft);
    //vfoDelete(sdr->vfo);
    //firDelete(sdr->bpf);
    ddcDelete(sdr->ddc);
    demodDelete(sdr->demodFm);
    demodDelete(sdr->demodAm);
    resamplerDelete(sdr->resampler);
    free(sdr);
    return TRUE;
}




/**
 */   
int sdrStart(SdrLib *sdr)
{
    pthread_t thread;
    if (sdr->running || sdr->device)
        {
        error("Device already started");
        return FALSE;
        }
    if (!sdr->deviceCount)
        {
        error("No devices found");
        return FALSE;
        }
    Device *d = sdr->devices[0];
    if (!d->open(d->ctx))
        {
        error("Could not start device");
        return FALSE;
        }
    sdr->device = d;
    d->setGain(d->ctx, 1.0);
    d->setCenterFrequency(d->ctx, 88700000.0);
    trace("starting");
    int rc = pthread_create(&thread, NULL, sdrReaderThread, (void *)sdr);
    if (rc)
        {
        error("ERROR; return code from pthread_create() is %d", rc);
        return FALSE;
        }
    trace("started");
    sdr->thread = thread;
    return TRUE;
}


/**
 */   
int sdrStop(SdrLib *sdr)
{
    sdr->running = 0;
    void *status;
    pthread_join(sdr->thread, &status);
    Device *d = sdr->device;
    if (d)
        {
        d->close(d->ctx);
        sdr->device = NULL;
        }
    return TRUE;
}

/**
 */   
double sdrGetCenterFrequency(SdrLib *sdr)
{
    Device *d = sdr->device;
    return (d) ? d->getCenterFrequency(d->ctx) : 0.0;
}


/**
 */   
int sdrSetCenterFrequency(SdrLib *sdr, double freq)
{
    Device *d = sdr->device;
    return (d) ? d->setCenterFrequency(d->ctx, freq) : 0;
}

/**
 */   
void sdrSetDdcFreqs(SdrLib *sdr, float vfoFreq, float pbLoOff, float pbHiOff)
{
    ddcSetFreqs(sdr->ddc, vfoFreq, pbLoOff, pbHiOff);
    float rate = ddcGetOutRate(sdr->ddc);
    resamplerSetInRate(sdr->resampler, rate);
}


/**
 */   
float sdrGetSampleRate(SdrLib *sdr)
{
    Device *d = sdr->device;
    return (d) ? d->getSampleRate(d->ctx) : 0.0;
}



/**
 */   
int sdrSetSampleRate(SdrLib *sdr, float rate)
{
    Device *d = sdr->device;
    return (d) ? d->setSampleRate(d->ctx, rate) : 0;
}



/**
 */   
float sdrGetRfGain(SdrLib *sdr)
{
    Device *d = sdr->device;
    return (d) ? d->getGain(d->ctx) : 0.0;
}



/**
 */   
int sdrSetRfGain(SdrLib *sdr, float gain)
{
    Device *d = sdr->device;
    return (d) ? d->setGain(d->ctx, gain) : 0;
}


/**
 */   
float sdrGetAfGain(SdrLib *sdr)
{
    Audio *a = sdr->audio;
    return (a) ? audioGetGain(a) : 0.0;
}



/**
 */   
int sdrSetAfGain(SdrLib *sdr, float gain)
{
    Audio *a = sdr->audio;
    return (a) ? audioSetGain(a, gain) : 0;
}



/**
 */   
void sdrSetPowerSpectrumFunc(SdrLib *sdr, PowerSpectrumFunc *func, void *ctx)
{
    sdr->psFunc = func;
    sdr->psFuncCtx = ctx;
}








#define READSIZE (8 * 16384)

static void fftOutput(unsigned int *vals, int size, void *ctx)
{
    SdrLib *sdr = (SdrLib *)ctx;
    PowerSpectrumFunc *psFunc = sdr->psFunc;
    void *psFuncCtx = sdr->psFuncCtx;
    if (psFunc) (*psFunc)(vals, size, psFuncCtx);
}


static void resamplerOutput(float *buf, int size, void *ctx)
{
    SdrLib *sdr = (SdrLib *)ctx;
    //trace("Push audio:%d", size);
    audioPlay(sdr->audio, buf, size);
}


static void demodOutput(float *buf, int size, void *ctx)
{
    SdrLib *sdr = (SdrLib *)ctx;
    //trace("Push audio:%d", size);
    resamplerUpdate(sdr->resampler, buf, size, resamplerOutput, sdr);
}

static void ddcOutput(float complex *data, int size, void *ctx)
{
    SdrLib *sdr = (SdrLib *)ctx;
    sdr->demod->update(sdr->demod, data, size, demodOutput, sdr);
}

static void *sdrReaderThread(void *ctx)
{
    SdrLib *sdr = (SdrLib *)ctx;
    Device *dev = sdr->device;
    
    sdr->running = 1;
    
    while (sdr->running && dev->isOpen(dev->ctx))
        {
        int size;
        float complex *data = (float complex *)dev->read(dev->ctx, &size);
        if (data)
            {
            fftUpdate(sdr->fft, data, size, fftOutput, sdr);
            ddcUpdate(sdr->ddc, data, size, ddcOutput, sdr);
            free(data);
            }
        }

    sdr->running = 0;
    return NULL;
}





