/**
 * Audio I/O definitions and implementations.
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
    di->setSampleRate(di->ctx, 1234567.0);
    di->setCenterFrequency(di->ctx, 93700000.0);
    
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
    Pa_Sleep(10* 1000);
    if (!sdrStop(sdr))
        {
        error("Failure initializing sdrlib");
        }
    if (!sdrClose(sdr))
        {
        error("Failure initializing sdrlib");
        }
        
        
    
    return TRUE;
}



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

