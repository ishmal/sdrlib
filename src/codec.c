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


#include "codec.h"

#include "private.h"




Codec *codecCreate()
{
    Codec *obj = (Codec *)malloc(sizeof(Codec));
    if (!obj)
        return NULL;
    memset(obj, 0, sizeof(Codec));
    int err;
    obj->enc = opus_encoder_create(48000, 1, OPUS_APPLICATION_AUDIO, &err);
    if (err != OPUS_OK || obj->enc==NULL)
        {
        free(obj);
        return NULL;
        }
    if (ogg_stream_init(&(obj->os), 1) < 0)
        {
        opus_encoder_destroy(obj->enc);
        free(obj);
        return NULL;
        }
    unsigned char head[19];
    strcpy(head, "OpusHead");
    head[8]  = 1;  //version
    head[9]  = 1;  //nr channels  1 or 2
    head[10] = 0;  //16 bits, pre-skip
    head[11] = 0;
    head[12] = 0x80;  //samplerate, 32 bits, little-endian
    head[13] = 0xbb;   //0xbb80 == 48000
    head[14] = 0;
    head[15] = 0;
    head[16] = 0;  //gain, 16 bits.  0 recommended
    head[17] = 0;
    head[18] = 0;
    ogg_packet op;
    op.packet     = head;
    op.bytes      = 19;
    op.b_o_s      = 1;
    op.e_o_s      = 0;
    op.granulepos = 0;
    op.packetno   = 0;
    ogg_stream_packetin(&(obj->os), &op);
    return obj;
}



void codecDelete(Codec *obj)
{
    if (obj)
        {
        opus_encoder_destroy(obj->enc);
        ogg_stream_clear(&(obj->os));
        free(obj);
        }
}



int codecEncode(Codec *obj, float *data, int datalen, ByteOutputFunc *func, void *context)
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
            ogg_packet op;
            op.packet = obj->enc_outbuf;
            op.bytes  = len;
            op.b_o_s=1;
            op.e_o_s=0;
            op.granulepos=0;
            op.packetno=0;
            ogg_stream_packetin(&(obj->os), &op);
            ogg_page page;
            if (ogg_stream_pageout(&(obj->os), &page))
                {
                if (func)
                    (*func)(obj->enc_outbuf, len, context);
                }
            }
        }
        
    obj->enc_inbuf_ptr = inptr;
    
    return TRUE;
}




