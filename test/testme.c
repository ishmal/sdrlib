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


void test_list_print(void *data)
{
    char *str = (char *) data;
    printf("str: %s\n", str);
}

int test_list()
{
    List *xs = listAppend(NULL, strdup("The"));
    listAppend(xs, strdup("The"));
    listAppend(xs, strdup("Quick"));
    listAppend(xs, strdup("Brown"));
    listAppend(xs, strdup("Fox"));

    listForEach(xs, test_list_print);
    
    listDelete(xs, free);

    return TRUE;
}


void test_device_close(void *data)
{
    Device *dv = (Device *)data;
    dv->close(dv->ctx);
}

int test_device()
{
    List *xs = deviceScan(DEVICE_SDR);
    
    int count = listLength(xs);
    trace("count:%d", count);
    

    int i=0;
    for ( ; i < 100000 ; i++)
        {
        }
        
    listForEach(xs, test_device_close);
    listDelete(xs, free);
    return TRUE;
}



int dotests()
{
    test_device();
    return TRUE;
}


int main(int argc, char **argv)
{
    dotests();
    return 0;
}

