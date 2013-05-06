/**
 * Simple command-line interface
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
#include <ctype.h>

#include <sdrlib.h>

#define BUFLEN 256

#include "private.h"


static int equ(char *a, char *b)
{
    return strcmp(a, b)==0;
}

static int getDouble(char *str, double *f)
{
    char *end;
    double val = strtod(str, &end);
    if (str == end)
        {
        error("Invalid floating point format: '%s'", str);
        return FALSE;
        }
    else
        {
        *f = val;
        return TRUE;
        }
 }

static int getFloat(char *str, float *f)
{
    double val;
    int ret = getDouble(str, &val);
    if (ret)
        *f = val;
    return ret;
 }


int parseAndExecute(SdrLib *sdr, char *buf)
{
    int len = strlen(buf);
    if (len > 0 && buf[len-1]=='\n')
        {
        len--;
        buf[len] = '\0';
        }
    int i = 0;
    for (i=0 ; i < len ; i++)
        buf[i] = tolower(buf[i]);
    trace("Buf:'%s'", buf);
    char *delim = " ,\t";
    char *ctx;
    char *cmd = strtok_r(buf, delim, &ctx);
    if (!cmd)
        return TRUE;
    char *p0 = strtok_r(NULL, delim, &ctx);
    
    if (equ(cmd, "exit")||equ(cmd, "quit")||equ(cmd, "q"))
        {
        return -1;
        }
    else if (equ(cmd, "start"))
        {
        sdrStart(sdr);
        }
    else if (equ(cmd, "stop"))
        {
        sdrStop(sdr);
        }
    else if (equ(cmd, "freq")||equ(cmd, "f"))
        {
        if (!p0)
            {
            double freq = sdrGetCenterFrequency(sdr);
            trace("freq: %f", freq);
            }
        else
            {
            double freq;
            if (getDouble(p0, &freq))
                {
                sdrSetCenterFrequency(sdr, freq);
                }
            }
        }
    else if (equ(cmd, "gain") || equ(cmd, "g"))
        {
        if (!p0)
            {
            float gain = sdrGetRfGain(sdr);
            trace("gain: %f", gain);
            }
        else
            {
            float gain;
            if (getFloat(p0, &gain))
                {
                if (gain < 0.0 || gain > 1.0)
                    {
                    error("Gain must be in the range 0.0 .. 1.0");
                    }
                else
                    sdrSetRfGain(sdr, gain);
                }
            }
        }
    else if (equ(cmd, "vfo") || equ(cmd, "v"))
        {
        if (!p0)
            {
            float f = sdrGetVfo(sdr);
            trace("vfo: %f", f);
            }
        else
            {
            float f;
            if (getFloat(p0, &f))
                {
                sdrSetVfo(sdr, f);
                }
            }
        }
    else if (equ(cmd, "lo"))
        {
        if (!p0)
            {
            float f = sdrGetPbLo(sdr);
            trace("pblo: %f", f);
            }
        else
            {
            float f;
            if (getFloat(p0, &f))
                {
                sdrSetPbLo(sdr, f);
                }
            }
        }
    else if (equ(cmd, "hi"))
        {
        if (!p0)
            {
            float f = sdrGetPbHi(sdr);
            trace("pbhi: %f", f);
            }
        else
            {
            float f;
            if (getFloat(p0, &f))
                {
                sdrSetPbHi(sdr, f);
                }
            }
        }
    else
        {
        error("Unimplemented command:'%s'", cmd);
        }

    return TRUE;
}


int cmdloop()
{
    SdrLib *sdr = sdrCreate(NULL, NULL);
    if (!sdr)
        return FALSE;
    char *inbuf = (char *)malloc(BUFLEN+1);
    while (1)
        {
        printf("sdr > ");
        fgets(inbuf, BUFLEN, stdin);
        int ret = parseAndExecute(sdr, inbuf);
        if (ret < 0)
            break;
        }
    sdrDelete(sdr);
    return TRUE;
}


int main(int argc, char **argv)
{
    cmdloop();
    return 0;
}

