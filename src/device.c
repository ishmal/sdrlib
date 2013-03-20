/**
 * API for adding devices to this lib
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
#include <dirent.h>
#include <dlfcn.h>
#include <limits.h>

#include "device.h"
#include "private.h"

static Parent parent;


static char *getExecutableDir()
{
    char pathName[PATH_MAX+1];
    if (!realpath(getprogname(), pathName))
        return strdup(".");
    char *pos = strrchr(pathName, '/');
    //trace("Program name is %s", pathName);
    if (!pos)
        return strdup(".");
    else
        {
        *pos = 0;
        return strdup(pathName);
        }
}

static char *getDeviceDir()
{
    char *dir = getExecutableDir();
    //trace("Program dir is %s", dir);
    int len = strlen(dir) + 8;
    char *deviceDir = (char *) malloc(len);
    strcpy(deviceDir, dir);
    strcat(deviceDir, "/device");
    //trace("Device dir is %s", deviceDir);
    free(dir);
    return deviceDir;
}

List *deviceScan(int type)
{
    parent.trace = trace;
    parent.error = error;
    List *xs = NULL;
    char *deviceDir = getDeviceDir();  
    int dirLen = strlen(deviceDir);
    DIR *dir = opendir(deviceDir);
    while (1)
        {
        struct dirent *de = readdir(dir);
        if (!de)
            break;
        if (de->d_type != DT_REG)
            continue;
        char *name = de->d_name;
        trace("name: '%s'", name);
        int fullLen = dirLen + 1 + strlen(name) + 1;
        char *fullName = (char *)malloc(fullLen);
        strcpy(fullName, deviceDir);
        strcat(fullName, "/");
        strcat(fullName, name);
        trace("full name: '%s'", fullName);
        
        void *dlib = dlopen(fullName, RTLD_LAZY);
        if (dlib)
            {
            trace("got dynamic lib");
            void *sym = dlsym(dlib, "deviceOpen");
            if (sym)
                {
                trace("got function");
                DeviceOpenFunc *func = (DeviceOpenFunc *)sym;
                Device *dev = (Device *)malloc(sizeof(Device));
                if (!dev)
                    {
                    error("creating Device info structure");
                    return xs;
                    }
                int ret = func(dev, &parent);
                if (ret)
                    {
                    trace("got device!!");
                    trace("Name: %s", dev->name);
                    xs = listAppend(xs, dev);
                    }
                else
                    {
                    free(dev);
                    }
                }
            }
        free(fullName);
        }  
    free(deviceDir);
    return xs;
    
}



