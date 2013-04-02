#ifndef _QUEUE_H_
#define _QUEUE_H_
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


#include <pthread.h>

#include "sdrlib.h"

typedef struct
{
    void *buf;
    int size;
} QueueItem;

struct Queue
{
    int head;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int tail;
    int size;
    int count;
    //We will allocate space for the Queue, plus 'size' number of QueueItems
    QueueItem queueItems[];
};

/**
 * Create a new Queue instance.
 * @return a new Queue instance
 */  
Queue *queueCreate(int size);

/**
 * Free up queue resources
 */
void queueDelete(Queue *queue);

/**
 * Add a buffer to the queue
 * @param queue a queue instance.
 */   
int queuePush(Queue *queue, void *buf, int size);

/**
 * Pull a buffer from the  queue
 * @param queue a queue instance.
 */   
void *queuePop(Queue *queue, int *size);



#endif /* _QUEUE_H_ */

