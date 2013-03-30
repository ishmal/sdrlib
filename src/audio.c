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
#include <sdrlib.h>

#include <portaudio.h>

#include "audio.h"
#include "private.h"

#define SAMPLE_RATE 44100



static int queuePush(Audio *ctx, float v)
{
    pthread_mutex_lock(&(ctx->queueMutex));
    while (ctx->queueSize >= SENDBUF_SIZE)
        pthread_cond_wait(&(ctx->queueCond), &(ctx->queueMutex));
    int head = (ctx->head + 1) & 0xfffff;
    //trace("head:%d tail:%d", head, ctx->tail);
    ctx->sendbuf[head] = v;
    ctx->queueSize++;
    ctx->head = head;
    pthread_cond_signal(&(ctx->queueCond));
    pthread_mutex_unlock(&(ctx->queueMutex));
    return TRUE;
}

static float queuePop(Audio *ctx)
{
    pthread_mutex_lock(&(ctx->queueMutex));
    while (ctx->queueSize== 0)
        pthread_cond_wait(&(ctx->queueCond), &(ctx->queueMutex));
    int tail = (ctx->tail + 1) & 0xfffff;
    float v = ctx->sendbuf[tail];
    ctx->queueSize--;
    ctx->tail = tail;
    pthread_cond_signal(&(ctx->queueCond));
    pthread_mutex_unlock(&(ctx->queueMutex));
    return v;
}


static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData)
{
    Audio *audio = (Audio *) userData;
    float *out = (float *)outputBuffer;
    while (framesPerBuffer--)
        {
        float v = queuePop(audio);
        //trace("v:%f",v);
        *out++ = v;
        *out++ = v;
        }
    return paContinue;
}


/**
 * Create a new Audio instance.
 * @return a new Audio instance
 */  
Audio *audioCreate()
{
    Audio *audio = (Audio *)malloc(sizeof(Audio));
    if (!audio)
        return audio;
    audio->sampleRate = (float)SAMPLE_RATE;
    pthread_mutex_init(&(audio->queueMutex), NULL);
    pthread_cond_init(&(audio->queueCond), NULL);
    audio->head = 1;
    audio->tail = 0;

    int err = Pa_Initialize();
    if ( err != paNoError )
        {
        error("audioCreate init: %s", Pa_GetErrorText(err) );
        free(audio);
        return NULL;
        }
    PaStreamParameters parms;
    parms.device = Pa_GetDefaultOutputDevice();
    parms.channelCount = 2;       /* stereo output */
    parms.sampleFormat = paFloat32; /* 32 bit floating point output */
    parms.hostApiSpecificStreamInfo = NULL;
 
    err = Pa_OpenStream(
            &(audio->stream),
            NULL, /* no input */
            &parms,
            SAMPLE_RATE,
            paFramesPerBufferUnspecified,
            paClipOff,      /* we won't output out of range samples so don't bother clipping them */
            paCallback,
            (void *)audio );
    if( err != paNoError )
        {
        error("audioCreate open: %s", Pa_GetErrorText(err) );
        Pa_Terminate();
        free(audio);
        return NULL;
        }
 
    err = Pa_StartStream( audio->stream );
    if( err != paNoError )
        {
        error("audioCreate start: %s", Pa_GetErrorText(err) );
        Pa_CloseStream(audio->stream);
        Pa_Terminate();
        free(audio);
        return NULL;
        }
 
    return audio;
}


/**
 *
 */
int audioPlay(Audio *audio, float *data, int size)
{
    while (size--)
        queuePush(audio, *data++);
     return TRUE;
}

/**
 * Delete an Audio instance, stopping
 * any processing and freeing any resources.
 * @param audio an Audio instance.
 */   
void audioDelete(Audio *audio)
{
    if (!audio)
        return;
        
    int err = Pa_StopStream(audio->stream);
    if (err != paNoError)
        error("audioDelete stop: %s", Pa_GetErrorText(err) );
    err = Pa_CloseStream(audio->stream);
    if (err != paNoError)
        error("audioDelete close: %s", Pa_GetErrorText(err) );
    err = Pa_Terminate();
    if ( err != paNoError )
        error("audioDelete terminate: %s", Pa_GetErrorText(err) );
    pthread_mutex_destroy(&(audio->queueMutex));
    pthread_cond_destroy(&(audio->queueCond));
    free(audio);
}

