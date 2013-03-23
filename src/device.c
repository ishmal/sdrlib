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
#include <limits.h>

#include "device.h"
#include "private.h"


static Parent parent;

#ifdef WIN32
#include <windows.h>
#define SEPCHR ('\\')
#define SEPSTR ("\\")
static char _prognamebuf[MAX_PATH];
static char *getprogname()
{
    GetModuleFileName(NULL, _prognamebuf, MAX_PATH-1);
    return _prognamebuf;
}

static int realpath(char *fname, char *fullpath)
{
    return GetFullPathName(fname, MAX_PATH-1, fullpath, NULL);
}

static void *dload(char *fname)
{
    return (void *)LoadLibrary(fname);
}

static void *dlsym(void *lib, char *funcname)
{
    return (void *)GetProcAddress(lib, funcname);
}
#else
#include <dlfcn.h>
#define SEPCHR ('/')
#define SEPSTR ("/")
static void *dload(char *fname)
{
    return (void *)dlopen(fname, RTLD_LAZY);
}

#endif


static char *getExecutableDir()
{
    char pathName[PATH_MAX+1];
    if (!realpath(getprogname(), pathName))
        return strdup(".");
    char *pos = strrchr(pathName, SEPCHR);
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
    strcat(deviceDir, SEPSTR);
    strcat(deviceDir, "device");
    //trace("Device dir is %s", deviceDir);
    free(dir);
    return deviceDir;
}

int deviceScan(int type, Device **outbuf, int maxDevices)
{
    parent.trace = trace;
    parent.error = error;
    
    char *deviceDir = getDeviceDir();  
    int dirLen = strlen(deviceDir);
    DIR *dir = opendir(deviceDir);
    if (!dir)
        {
        error("Devices directory '%s' not found", deviceDir);
        return 0;
        }
    int count = 0;
    while (count < maxDevices)
        {
        struct dirent *de = readdir(dir);
        if (!de)
            break;
        char *name = de->d_name;
        trace("name: '%s'", name);
        int fullLen = dirLen + 1 + strlen(name) + 1;
        char *fullName = (char *)malloc(fullLen);
        strcpy(fullName, deviceDir);
        strcat(fullName, SEPSTR);
        strcat(fullName, name);
        trace("full name: '%s'", fullName);
        
        void *dlib = dload(fullName);
        if (dlib)
            {
            trace("got dynamic lib");
            void *sym = dlsym(dlib, "deviceCreate");
            if (sym)
                {
                trace("got function");
                DeviceOpenFunc *func = (DeviceOpenFunc *)sym;
                Device *dev = (Device *)malloc(sizeof(Device));
                if (!dev)
                    {
                    error("creating Device info structure");
                    return count;
                    }
                int ret = func(dev, &parent);
                if (ret)
                    {
                    trace("got device!!");
                    trace("Name: %s", dev->name);
                    outbuf[count++] = dev;
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
    return count;
}



