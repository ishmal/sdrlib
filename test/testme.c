/**
 * Placeholder for writing and executing some tests
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
#include <string.h>
#include <stdlib.h>
#include <sdrlib.h>


#include "audio.h"
#include "device.h"
#include "private.h"

int test_audio()
{
    Audio *audio = audioCreate();
    if (!audio)
        error("test fail");
    else
        trace("test success");
    audioDelete(audio);
    return TRUE;
}




int test_device()
{
    Device *devices[20];
    int count = deviceScan(DEVICE_SDR, devices, 20);
    
    trace("count:%d", count);
    
    Device *di = devices[0];
    
    di->setGain(di->ctx, 1.0);
    di->setSampleRate(di->ctx, 2048000.0);
    di->setCenterFrequency(di->ctx, 88700000.0);
    
    trace("Name      : %s", di->name);
    trace("Gain      : %f", di->getGain(di->ctx));
    trace("SampleRate: %f", di->getSampleRate(di->ctx));
    trace("CenterFreq: %f", di->getCenterFrequency(di->ctx));

    int i=0;
    for ( ; i < 100000 ; i++)
        {
        }
        
    return TRUE;
}

int test_main()
{
    SdrLib *sdr = sdrCreate();    
    if (!sdr)
        {
        error("Failure initializing sdrlib");
        return FALSE;
        }
    if (!sdrStart(sdr))
        {
        error("Failure initializing sdrlib");
        }
    Pa_Sleep(100* 1000);
    if (!sdrStop(sdr))
        {
        error("Failure initializing sdrlib");
        }
    if (!sdrDelete(sdr))
        {
        error("Failure initializing sdrlib");
        }
        
        
    
    return TRUE;
}


#if 0

static void test_ws1()
{
    unsigned char hash[20];
    char *str = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    //hash should be:  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1 
    sha1hash((unsigned char *)str, strlen(str), hash);
    int i;
    for (i = 0 ; i < 20 ; i++)
        printf("%02x", hash[i]);
    printf("\n");
}


static void test_ws2()
{
    char b64buf[256];
    unsigned char plainbuf[256];
    strcpy(plainbuf, "the quick brown fox jumped over the lazy dog");
    base64encode(plainbuf, strlen(plainbuf), b64buf);
    printf("%d : '%s'\n", (int)strlen(b64buf), b64buf);
    base64decode(b64buf, plainbuf, 256);
    printf("%d : '%s'\n", (int)strlen(plainbuf), plainbuf);
}

static void test_ws3()
{
    char *key = "x3JJHMbDL1EzLkh9GBhXDw==";
    char buf[256];
    snprintf(buf, 256, "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", key);
    char result[32];
    sha1hash64(buf, strlen(buf), result);
    char *exp = "HSmrc0sMlYUkAGmm5OPpG2HaGWk=";
    printf("result: '%s'   should be: '%s'\n", result, exp);
    if (strcmp(result, exp)==0)
        printf("success\n");
    else
        printf("failure\n");
}

#endif


int dotests()
{
    test_main();
    return TRUE;
}


int main(int argc, char **argv)
{
    dotests();
    return 0;
}

