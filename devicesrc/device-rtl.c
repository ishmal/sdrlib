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

#include <stdlib.h>
#include <complex.h>
#include <rtl-sdr.h>

#include "device.h"

typedef struct
{
    unsigned char *readbuf;
    rtlsdr_dev_t *dev;
    complex *lut;
} Context;

#define DEV (((Context*)ctx)->dev)

static void setSampleRate(void *ctx, float rate)
{
    rtlsdr_set_sample_rate(DEV, (int)rate);
}

static float getSampleRate(void *ctx)
{
    return (float) rtlsdr_get_sample_rate(DEV);
}

static void setCenterFrequency(void *ctx, double freq)
{
    rtlsdr_set_center_freq(DEV, (uint32_t)freq);
}

static double getCenterFrequency(void *ctx)
{
    uint32_t f = rtlsdr_get_center_freq(DEV);
    return (double)f;
}


static int read(void *ctxt, float complex *cbuf, int len)
{
    Context *ctx = (Context *) ctxt;
    unsigned char *bbuf = ctx->readbuf;
    int count;
    rtlsdr_read_sync(ctx->dev, bbuf, len<<1, &count);
    float complex *b = cbuf;
    int i=0;
    while (i<count)
        {
        int hi = bbuf[i++];
        int lo = bbuf[i++];
        *b++ = ctx->lut[(hi<<8) + lo];
        }
    return count>>1;
}

static Context *startup()
{
    Context *ctx = (Context *)malloc(sizeof(Context));
    if (!ctx)
        return NULL;
    ctx->readbuf = (unsigned char *)malloc(1000000);
    ctx->lut = (complex *)malloc(256 * 256 * sizeof(complex));
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
    rtlsdr_dev_t *dev;
    int ret = rtlsdr_open(&dev, idx);
    ctx->dev = dev;
    return ctx;
}

static void shutdown(Context *ctx)
{
    //do shutdowny things here
    rtlsdr_close(ctx->dev);
    free(ctx->readbuf);
    free(ctx->lut);
    free(ctx);   
}


static void deviceDelete(Device *dv)
{
    shutdown((Context *)dv->ctx);
    free(dv);
}

Device *deviceCreate()
{
    Device *dv = (Device *)malloc(sizeof(Device));
    if (!dv)
        return NULL;
    Context *ctx = startup();
    if (!ctx)
        {
        free(dv);
        return NULL;
        }
    dv->type = DEVICE_SDR,
    dv->name               = "RTL - SDR Device";
    dv->ctx                = (void *)ctx;
    dv->delete             = deviceDelete;
    dv->setSampleRate      = setSampleRate;
    dv->getSampleRate      = getSampleRate;
    dv->setCenterFrequency = setCenterFrequency;
    dv->getCenterFrequency = getCenterFrequency;
    dv->read               = read;
    return dv;
}

