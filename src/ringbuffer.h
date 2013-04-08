/*
 * Lockfree high-throughput inter-thread ringbuffer.
 * Written by Elias Önal <EliasOenal@gmail.com>, released as public domain.
 *
 * Bob Jamison:  replaced ringbuffer_init() with create() and delete().
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#include <signal.h>

typedef struct {
	volatile sig_atomic_t total_size;
	volatile sig_atomic_t element_size;
	volatile sig_atomic_t head;
	volatile sig_atomic_t tail;
	volatile unsigned char elems[1];
} ringbuffer;

ringbuffer *ringbuffer_create(int elem_count, int element_size);
void ringbuffer_delete(ringbuffer *rb);
int ringbuffer_is_empty(volatile const ringbuffer *rb);
int ringbuffer_is_full(volatile const ringbuffer *rb);
int ringbuffer_write(volatile ringbuffer *rb, const void *element);
void *ringbuffer_wpeek(volatile ringbuffer *rb);
int ringbuffer_read(volatile ringbuffer *rb, void *element);
void *ringbuffer_rpeek(volatile ringbuffer *rb);

#endif
