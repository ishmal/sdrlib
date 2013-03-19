#ifndef _PLUGIN_H_
#define _PLUGIN_H_

/**
 * API for adding things to this lib
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

#include <complex.h>
#include "util.h"

typedef enum
{
    PLUGIN_NONE,
    PLUGIN_SDR
} PluginType;




typedef struct SdrPlugin SdrPlugin;

struct SdrPlugin
{
    int    type;
    char   *name;
    void   *ctx;
    void   (*delete)(SdrPlugin *pi);
    
    void   (*setSampleRate)(void *ctx, float rate);
    float  (*getSampleRate)(void *ctx);
    void   (*setCenterFrequency)(void *ctx, double freq);
    double (*getCenterFrequency)(void *ctx);
    int    (*read)(void *ctx, float complex *buf, int len);
};


List *pluginScan(int type);


typedef SdrPlugin* PluginCreateFunc();



#endif /* _PLUGIN_H_ */


