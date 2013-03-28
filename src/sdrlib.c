/**
 * This is intended as a small and simple
 * SDR library with as low code complexity, 
 * and as few dependencies as possible.  
 * 
 * The goal is to make code readability, maintainability
 * and portability as high as possible.  
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
#include <sdrlib.h>


#include "impl.h"

/**
 */  
SdrLib *sdrCreate()
{
    SdrLib *inst = (SdrLib *)malloc(sizeof(SdrLib));
    if (!inst)
        return NULL;
    if (!implCreate(inst))
        {
        free(inst);
        return NULL;
        }
    return inst;
}

/**
 */   
int sdrDelete(SdrLib *sdrlib)
{
    int ret = implDelete((Impl *)sdrlib->impl);
    free(sdrlib);
    return ret;
}



/**
 */   
int sdrStart(SdrLib *sdrlib)
{
    return implStart((Impl *)sdrlib->impl);
}


/**
 */   
int sdrStop(SdrLib *sdrlib)
{
    return implStop((Impl *)sdrlib->impl);
}

/**
 */   
double sdrGetCenterFrequency(SdrLib *sdrlib)
{
    return implGetCenterFrequency((Impl *)sdrlib->impl);
}


/**
 */   
int sdrSetCenterFrequency(SdrLib *sdrlib, double freq)
{
    return implSetCenterFrequency((Impl *)sdrlib->impl, freq);
}

/**
 */   
float sdrGetGain(SdrLib *sdrlib)
{
    return implGetGain((Impl *)sdrlib->impl);
}


/**
 */   
int sdrSetGain(SdrLib *sdrlib, float gain)
{
    return implSetGain((Impl *)sdrlib->impl, gain);
}

void sdrSetPowerSpectrumFunc(SdrLib *sdrlib, PowerSpectrumFunc *func, void *ctx)
{
    Impl *impl = (Impl *)sdrlib->impl;
    impl->psFunc = func;
    impl->psFuncCtx = ctx;
}


