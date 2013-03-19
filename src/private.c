/**
 * Non-public definitions.
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
#include <stdarg.h>

#include "private.h"


void trace(char *format, ...)
{
    fprintf(stderr, "trace: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void error(char *format, ...)
{
    fprintf(stderr, "err: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}




List *listAppend(List *list, void *data)
{
    List *node = (List *)malloc(sizeof(List));
    if (!node)
        return NULL;
    node->next = NULL;
    node->data = data;
    if (list)
        {
        while (list->next)
            list = list->next;
        list->next = node;
        }
    return node;
}

List *listRemove(List *list, void *data)
{
    List *curr = list;
    List *next = NULL;
    List *prev = NULL;
    while (curr)
        {
        next = curr->next;
        if (list->data == data)
            {
            free(curr);
            if (prev)
                prev->next = next;
            else
                list = next;
            }
        prev = curr;
        curr = next;
        }
    return list;
}


void listForEach(List *list, ListFunc func)
{
    for ( ; list ; list=list->next)
        func(list->data);
}

void listDelete(List *list, ListFunc func)
{
    List *curr = list;
    List *next = NULL;
    while (curr)
        {
        next = curr->next;
        if (func)
            func(curr->data);
        free(curr);
        curr = next;
        }
}





