#ifndef _AUDIO_H_
#define _AUDIO_H_

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


#include <portaudio.h>
#include <pthread.h>

#define SENDBUF_SIZE (0xfffff + 1)  

typedef struct
{
    PaStream *stream;
    float sampleRate;
    float sendbuf[SENDBUF_SIZE];
    pthread_mutex_t queueMutex;
    pthread_cond_t  queueCond;
    int queueSize;
    int head;
    int tail;
} Audio;


/**
 * Create a new Audio instance.
 * @return a new Audio instance
 */  
Audio *audioCreate();

/**
 * Send audio data to the player
 */
int audioPlay(Audio *audio, float *data, int size);

/**
 * Delete an Audio instance, stopping
 * any processing and freeing any resources.
 * @param audio an Audio instance.
 */   
void audioDelete(Audio *audio);



#endif /* _AUDIO_H_ */

