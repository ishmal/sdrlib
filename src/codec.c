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



static void sendHeader(ogg_stream_state *os)
{
    ogg_stream_reset(os);
    unsigned char head[] = {
        'O', 'p', 'u', 's', 'H', 'e', 'a', 'd', 
        1,                //version
        1,                //nr channels  1 or 2
        0, 0,             //16 bits, pre-skip
        0x80, 0xbb, 0, 0, //samplerate, 32 bits, little-endian. 0xbb80 == 48000
        0, 0,             //gain, 16 bits.  0 recommended
        0                 // mapping, 0=single stream
    };
    ogg_packet op;
    op.packet     = head;
    op.bytes      = sizeof(head);
    op.b_o_s      = 1;
    op.e_o_s      = 0;
    op.granulepos = 0;
    op.packetno   = 0;  //currently ignored by libogg
    ogg_stream_packetin(os, &op);
    unsigned char tags[] = {
        'O', 'p', 'u', 's', 'T', 'a', 'g', 's', 
        6, 0, 0, 0,       //length of vendor string
        'l','i','b','s','d','r', //vendor (us)
        0,0,0,0             //count of tag strings
    };
    op.packet     = tags;
    op.bytes      = sizeof(tags);
    op.b_o_s      = 1;
    op.e_o_s      = 0;
    op.granulepos = 0;
    op.packetno   = 0;  //currently ignored by libogg
    ogg_stream_packetin(os, &op);
}




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
    sendHeader(&(obj->os));
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


static void dumpBuf(unsigned char *buf, int len)
{
    FILE *f = fopen("dump.opus", "wb");
    fwrite(buf, 1, len, f);
    fclose(f);
}



int codecEncode(Codec *obj, float *data, int datalen, ByteOutputFunc *func, void *context)
{
    float *inbuf = obj->inbuf;
    int inptr    = obj->inbufPtr;
    
    while (datalen--)
        {
        inbuf[inptr++] = *data++;
        if (inptr >= FRAME_SIZE)
            {
            inptr = 0;
            int len = opus_encode_float(obj->enc, inbuf, FRAME_SIZE, obj->opusbuf, OPUS_PACKET);
            ogg_packet op;
            op.packet = obj->opusbuf;
            op.bytes  = len;
            op.b_o_s=0;
            op.e_o_s=0;
            op.granulepos=0;
            op.packetno=0; //currently ignored by libogg
            ogg_stream_packetin(&(obj->os), &op);
            obj->packetCount++;
            if (obj->packetCount >= 16)
                {
                obj->packetCount = 0;
                ogg_packet op;
                op.packet = obj->opusbuf;
                op.bytes  = 0;
                op.b_o_s=0;
                op.e_o_s=1;
                op.granulepos=0;
                op.packetno=0; //currently ignored by libogg
                ogg_stream_packetin(&(obj->os), &op);
                ogg_page page;
                unsigned char *buf = obj->oggbuf;
                unsigned char *b = buf;
                while (ogg_stream_flush(&(obj->os), &page))
                    {
                    memcpy(b, page.header, page.header_len);
                    b += page.header_len;
                    memcpy(b, page.body, page.body_len);
                    b += page.body_len;
                    }
                int bufsize = b - buf;
                for (int i = 0 ; i < 50 ; i++)
                    printf("%d : %02x %c\n", i, buf[i], buf[i]);
                dumpBuf(buf, bufsize);
                if (func)
                    (*func)(buf, bufsize, context);
                sendHeader(&(obj->os));
                }
            }
        }
        
    obj->inbufPtr = inptr;
    
    return TRUE;
}




