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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>

#include "plugin.h"
#include "private.h"


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

static char *getPluginDir()
{
    char *dir = getExecutableDir();
    trace("Program dir is %s", dir);
    int len = strlen(dir) + 8;
    char *pluginDir = (char *) malloc(len);
    strcpy(pluginDir, dir);
    strcat(pluginDir, "/plugin");
    trace("Plugin dir is %s", pluginDir);
    free(dir);
    return pluginDir;
}

List *pluginScan(int type)
{
    char *pluginDir = getPluginDir();  
    int dirLen = strlen(pluginDir);
    DIR *dir = opendir(pluginDir);
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
        strcpy(fullName, pluginDir);
        strcat(fullName, "/");
        strcat(fullName, name);
        trace("full name: '%s'", fullName);
        free(fullName);
        }  
    free(pluginDir);
    return NULL;
    
}



