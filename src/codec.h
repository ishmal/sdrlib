/**
 * Audio codec definitions and implementations.
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

#include <string.h>


#include <opus.h>

typedef struct Codec Codec;


#define MAX_PACKET (1024 * 16)
#define FRAME_SIZE (2880)
struct Codec
{
    OpusEncoder *enc;
    float enc_inbuf[FRAME_SIZE];
    int enc_inbuf_ptr;
    unsigned char enc_outbuf[MAX_PACKET];
};




/**
 *
 */
Codec *codecCreate();



/**
 *
 */
void codecDelete(Codec *obj);


/**
 *
 */
int codecEncode(Codec *obj, float *data, int datalen);




