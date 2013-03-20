/**
 * Sample plugin
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
#include <complex.h>
#include <rtl-sdr.h>

#include "device.h"

typedef struct
{
    rtlsdr_dev_t *dev;
    float complex *lut;
    float gainscale;
    unsigned char *readbuf;
    Parent *par;
} Context;

#define DEV (((Context*)ctx)->dev)
#define LUT (((Context*)ctx)->lut)
#define GAINSCALE (((Context*)ctx)->gainscale)
#define READBUF (((Context*)ctx)->readbuf)


static int setGain(void *ctx, float gain)
{
    //the int param for rtl gain is tenths of a dB
    int dgain = (int)(gain * GAINSCALE);
    int ret = rtlsdr_set_tuner_gain(DEV, dgain);
    return !ret;
}

static float getGain(void *ctx)
{
    int dgain = rtlsdr_get_tuner_gain(DEV);
    return ((float)dgain) / GAINSCALE;
}

static int setSampleRate(void *ctx, float rate)
{
    int ret = rtlsdr_set_sample_rate(DEV, (int)rate);
    return !ret;
}

static float getSampleRate(void *ctx)
{
    return (float) rtlsdr_get_sample_rate(DEV);
}

static int setCenterFrequency(void *ctx, double freq)
{
    int ret = rtlsdr_set_center_freq(DEV, (uint32_t)freq);
    return !ret;
}

static double getCenterFrequency(void *ctx)
{
    uint32_t f = rtlsdr_get_center_freq(DEV);
    return (double)f;
}


static int read(void *ctx, float complex *cbuf, int buflen)
{
    unsigned char *bbuf = READBUF;
    float complex *lut = LUT;
    int count;
    rtlsdr_read_sync(DEV, bbuf, buflen<<1, &count);
    float complex *b = cbuf;
    int i=0;
    while (i<count)
        {
        int hi = bbuf[i++];
        int lo = bbuf[i++];
        *b++   = lut[(hi<<8) + lo];
        }
    return count>>1;
}

static int write(void *ctx, float complex *cbuf, int datalen)
{
    return 0;
}

static int transmit(void *ctx, int truefalse)
{
    return 0;
}

static Context *startup(Parent *par)
{
    rtlsdr_dev_t *dev = NULL;
    int devCount = rtlsdr_get_device_count();
    par->trace("devices:%d", devCount);
    int ret = rtlsdr_open(&dev, 0);
    if (!dev)
        {
        par->error("Could not open device");
        return NULL;
        }
    ret = rtlsdr_set_tuner_gain_mode(dev, 1);
    int count = rtlsdr_get_tuner_gains(dev, NULL);
    int *gains = (int *)malloc(count * sizeof(int));
    ret = rtlsdr_get_tuner_gains(dev, gains);
    int higain = gains[count - 1];
    par->trace("hi gain: %d", higain);

    Context *ctx = (Context *)malloc(sizeof(Context));
    if (!ctx)
        return NULL;
    ctx->gainscale = (float)higain;
    ctx->dev = dev;
    ctx->readbuf = (unsigned char *)malloc(1000000);
    ctx->lut = (float complex *)malloc(256 * 256 * sizeof(float complex));
    int idx = 0;
    int hi,lo;
    for (hi = 0 ; hi < 256 ; hi++)
        {
        float hival = ((float)(hi - 127)) / 128.0; 
        for (lo = 0; lo < 256 ; lo++)
            {
            float loval = ((float)(lo - 127)) / 128.0;
            ctx->lut[idx++] = hival+loval*I; 
            }    
        }
    return ctx;
}

static void shutdown(void *context)
{
    Context *ctx = (Context *)context;
    //do shutdowny things here
    rtlsdr_close(ctx->dev);
    free(ctx->readbuf);
    free(ctx->lut);
    free(ctx);   
}


int deviceOpen(Device *dv, Parent *parent)
{
    Context *ctx = startup(parent);
    if (!ctx)
        {
        free(dv);
        return 0;
        }
    dv->type = DEVICE_SDR,
    dv->name               = "RTL - SDR Device";
    dv->ctx                = (void *)ctx;
    dv->close              = shutdown;
    dv->setGain            = setGain;
    dv->getGain            = getGain;
    dv->setSampleRate      = setSampleRate;
    dv->getSampleRate      = getSampleRate;
    dv->setCenterFrequency = setCenterFrequency;
    dv->getCenterFrequency = getCenterFrequency;
    dv->read               = read;
    dv->write              = write;
    dv->transmit           = transmit;
    return 1;
}

