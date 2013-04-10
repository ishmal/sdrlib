// See header for information
#include <ringbuffer.h>
#include <stdlib.h>
#include <string.h>

ringbuffer *ringbuffer_create(int element_count, int element_size)
{
    int total_size = element_count * element_size;
    
    ringbuffer *rb = (ringbuffer *)malloc(sizeof(ringbuffer) + total_size);
    if (!rb)
        return NULL;
    
	rb->total_size = total_size;
	rb->element_size = element_size;
	rb->head = 0;
	rb->tail = 0;
	return rb;
}

void ringbuffer_delete(ringbuffer *rb)
{
    free(rb);
}

int ringbuffer_is_full(volatile const ringbuffer *rb)
{
	return (rb->head + rb->element_size) % rb->total_size == rb->tail;
}

int ringbuffer_is_empty(volatile const ringbuffer *rb)
{
	return rb->head == rb->tail;
}

int ringbuffer_write(volatile ringbuffer *rb, const void *element)
{
    int newhead = (rb->head + rb->element_size) % rb->total_size;
    if (newhead == rb->tail)
        return 0;
	memcpy((void*)&rb->elems[rb->head], (void*)element, rb->element_size);
	rb->head = newhead;
	return 1;
}

void *ringbuffer_wpeek(volatile ringbuffer *rb)
{
    int newhead = (rb->head + rb->element_size) % rb->total_size;
    if (newhead == rb->tail)
        return NULL;
	return (void*)&rb->elems[rb->head];
}

void ringbuffer_wadvance(volatile ringbuffer *rb)
{
    rb->head = (rb->head + rb->element_size) % rb->total_size;
}

int ringbuffer_read(volatile ringbuffer *rb, void *element)
{
    if (rb->head == rb->tail)
        return 0;
	memcpy((void*)element, (void*)&rb->elems[rb->tail], rb->element_size);
	rb->tail = (rb->tail + rb->element_size) % rb->total_size;
	return 1;
}


void *ringbuffer_rpeek(volatile ringbuffer *rb)
{
    if (rb->tail == rb->head)
        return NULL;
	return (void*)&(rb->elems[rb->tail]);
}


void ringbuffer_radvance(volatile ringbuffer *rb)
{
    rb->tail = (rb->tail + rb->element_size) % rb->total_size;
}


