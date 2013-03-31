/**
 * A queue of buffers
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

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "queue.h"

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/**
 * Create a new Queue instance.
 * @return a new Queue instance
 */  
Queue *queueCreate(int size)
{
    int allocSize = sizeof(Queue) + size * sizeof(QueueItem);
    Queue *queue = (Queue *)malloc(allocSize);
    if (!queue)
        return NULL;
    memset(queue, 0, allocSize);
    pthread_cond_init(&(queue->cond), NULL);
    pthread_mutex_init(&(queue->mutex), NULL);
    return queue;
}

/**
 * Free up queue resources
 */
void queueDelete(Queue *queue)
{
    if (queue)
        {
        while (queue->count > 0)
            {
            int size;
            QueueItem *item = queuePop(queue, &size);
            if (item)
                free(item->buf);
            }
        pthread_cond_destroy(&(queue->cond));
        pthread_mutex_destroy(&(queue->mutex));
        free(queue);
        }
}

/**
 * Add a buffer to the queue
 * @param queue a queue instance.
 */   
int queuePush(Queue *queue, void *buf, int size)
{
    pthread_mutex_lock(&(queue->mutex));
    while (queue->count >= queue->size)
        pthread_cond_wait(&(queue->cond),&(queue->mutex));
    int head = (queue->head + 1) % queue->size;
    QueueItem *qi = queue->buf + head;
    qi->buf = buf;
    qi->size = size;
    queue->head = head;
    queue->count++;
    pthread_cond_signal(&(queue->cond));
    pthread_mutex_unlock(&(queue->mutex));
    return TRUE;
}

/**
 * Add a buffer to the queue
 * @param queue a queue instance.
 */   
void *queuePop(Queue *queue, int *size)
{
    pthread_mutex_lock(&(queue->mutex));
    while (queue->count <= 0)
        pthread_cond_wait(&(queue->cond),&(queue->mutex));
    int tail = (queue->tail + 1) % queue->size;
    QueueItem *qi = queue->buf + tail;
    void *buf = qi->buf;
    *size = qi->size;
    queue->count--;    
    pthread_cond_signal(&(queue->cond));
    pthread_mutex_unlock(&(queue->mutex));
    return buf;        
}




