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

#include "util.h"

typedef enum
{
    PLUGIN_NONE,
    PLUGIN_SDR
} PluginType;



typedef struct
{
    int    type;
    void   (*setSampleRate)(float rate);
    float  (*getSampleRate)();
    void   (*setCenterFrequency)(double freq);
    double (*getCenterFrequency)();
} SdrPlugin;


List *pluginScan(int type);


#endif /* _PLUGIN_H_ */


