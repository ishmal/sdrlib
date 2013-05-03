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
#include <stdlib.h>

#include <string.h>


#include <opus/opus.h>

#include "private.h"

typedef struct Opus Opus;


#define MAX_PACKET (1024 * 16)
#define FRAME_SIZE (2880)
struct Opus
{
    OpusEncoder *enc;
    float enc_inbuf[FRAME_SIZE];
    int enc_inbuf_ptr;
    unsigned char enc_outbuf[MAX_PACKET];
};





Opus *opsuCreate()
{
    Opus *obj = (Opus *)malloc(sizeof(Opus));
    if (!obj)
        return NULL;
    memset(obj, 0, sizeof(Opus));
    int err;
    obj->enc = opus_encoder_create(48000, 1, OPUS_APPLICATION_AUDIO, &err);
    if (err != OPUS_OK || obj->enc==NULL)
        {
        free(obj);
        return NULL;
        }
    
    return obj;
}



void opusDelete(Opus *obj)
{
    if (obj)
        {
        opus_encoder_destroy(obj->enc);
        free(obj);
        }
}



int opusEncode(Opus *obj, float *data, int datalen)
{
    float *inbuf = obj->enc_inbuf;
    int inptr = obj->enc_inbuf_ptr;
    
    while (datalen--)
        {
        inbuf[inptr++] = *data++;
        if (inptr >= FRAME_SIZE)
            {
            inptr = 0;
            int len = opus_encode_float(obj->enc, inbuf, FRAME_SIZE, obj->enc_outbuf, MAX_PACKET);
            //do something with the outbuf
            }
        }
        
    obj->enc_inbuf_ptr = inptr;
    
    return TRUE;
}




