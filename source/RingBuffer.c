/**
 * @file ConcurrentRingBuffer.h
 * @author Nikola Spiric <nikola.spiric@rt-rk.com>
 */

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "RingBuffer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define RING_BUFFER_MAGIC (0xA354BDDD )

#ifndef MIN
#define MIN(x, y) ( (x) < (y) ? (x) : (y) )
#endif

#define VALID_HANDLE(handle) ( (handle) != NULL && (handle)->magic == RING_BUFFER_MAGIC )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

struct base_t {
    uint32_t head;
    uint32_t tail;
    uint32_t size;
};
// </ringbuf_t>

struct ringbuf_t {
    uint32_t magic;
    uint8_t* buffer;
    struct base_t* base;
    int sharedMemory;
};
// </ringbuf_t>

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static const uint8_t* RingBuffer_getEnd(const RingBuffer rb);

static uint8_t* RingBuffer_nextp(RingBuffer rb, const uint8_t *p);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

RingBuffer RingBuffer_fromSharedMemory(void* vptr, uint32_t size, int init) {
    RingBuffer rb = (RingBuffer)malloc(sizeof(struct ringbuf_t));

    memset(rb, 0x00, sizeof(struct ringbuf_t));

    uint8_t* data = (uint8_t*)vptr;

    rb->base = (struct base_t*)data;
    rb->base->size = size - sizeof(struct base_t);
    rb->buffer = data + sizeof(struct base_t);
    rb->magic = RING_BUFFER_MAGIC;
    rb->sharedMemory = 1;

    if(init) {
        RingBuffer_clear(rb);
    }

    return rb;
}

RingBuffer RingBuffer_new(uint32_t capacity) {
    RingBuffer rb = (RingBuffer)malloc(sizeof(struct ringbuf_t));

    memset(rb, 0x00, sizeof(struct ringbuf_t));

    // One byte is used for detecting the full condition.
    rb->base = (struct base_t*)malloc(sizeof(struct base_t));
    rb->base->size = capacity + 1;
    rb->buffer = (uint8_t*)malloc(rb->base->size);
    rb->magic = RING_BUFFER_MAGIC;
    rb->sharedMemory = 0;

    RingBuffer_clear(rb);

    return rb;
}

int32_t RingBuffer_free(RingBuffer* rb) {
    if(!VALID_HANDLE(*rb)) {
        return -1;
    }

    if(!(*rb)->sharedMemory) {
        free((*rb)->buffer);
        free((*rb)->base);
    }

    free(*rb);
    *rb = NULL;

    return 0;
}

int32_t RingBuffer_getSize(const RingBuffer rb) {
    if(!VALID_HANDLE(rb)) {
        return -1;
    }

    return rb->base->size;
}

int32_t RingBuffer_clear(RingBuffer rb) {
    if(!VALID_HANDLE(rb)) {
        return -1;
    }

    rb->base->head = rb->base->tail = 0;

    return 0;
}

int32_t RingBuffer_getCapacity(const RingBuffer rb) {
    if(!VALID_HANDLE(rb)) {
        return -1;
    }

    return RingBuffer_getSize(rb) - 1;
}

const uint8_t* RingBuffer_getEnd(const RingBuffer rb) {
    if(!VALID_HANDLE(rb)) {
        return NULL;
    }

    return rb->buffer + RingBuffer_getSize(rb);
}

int32_t RingBuffer_getBytesFree(const RingBuffer rb) {
    if(!VALID_HANDLE(rb)) {
        return -1;
    }

    if(rb->base->head >= rb->base->tail) {
        return RingBuffer_getCapacity(rb) - (rb->base->head - rb->base->tail);
    } else {
        return rb->base->tail - rb->base->head - 1;
    }
}

int32_t RingBuffer_getBytesUsed(const RingBuffer rb) {
    if(!VALID_HANDLE(rb)) {
        return -1;
    }

    return RingBuffer_getCapacity(rb) - RingBuffer_getBytesFree(rb);
}

int32_t RingBuffer_isFull(const RingBuffer rb) {
    if(!VALID_HANDLE(rb)) {
        return -1;
    }

    return RingBuffer_getBytesFree(rb) == 0;
}

int32_t RingBuffer_isEmpty(const RingBuffer rb) {
    if(!VALID_HANDLE(rb)) {
        return -1;
    }

    return RingBuffer_getBytesFree(rb) == RingBuffer_getCapacity(rb);
}

uint8_t* RingBuffer_nextp(RingBuffer rb, const uint8_t *p) {
    if(!VALID_HANDLE(rb)) {
        return NULL;
    }

    return rb->buffer + ((++p - rb->buffer) % RingBuffer_getSize(rb));
}

int32_t RingBuffer_write(RingBuffer dst, const void *src, uint32_t count) {
    if(!VALID_HANDLE(dst)) {
        return -1;
    }

    const uint8_t* u8src = (const uint8_t *)src;
    const uint8_t* bufend = RingBuffer_getEnd(dst);
    int overflow = (int32_t)count > RingBuffer_getBytesFree(dst);
    uint32_t nread = 0;

    while(nread != count) {
        // Don't copy beyond the end of the buffer
        uint32_t n = MIN(bufend - (dst->buffer + dst->base->head), count - nread);
        memcpy((dst->buffer + dst->base->head), u8src + nread, n);
        dst->base->head += n;
        nread += n;

        // Wrap ?
        if((dst->buffer + dst->base->head) == bufend) {
            dst->base->head = 0;
        }
    }
    if(overflow) {
        dst->base->tail = RingBuffer_nextp(dst, (dst->buffer + dst->base->head)) - dst->buffer;
    }

    return count;
}

int32_t RingBuffer_read(RingBuffer src, void *dst, uint32_t count) {
    if(!VALID_HANDLE(src)) {
        return -1;
    }

    uint32_t bytes_used = RingBuffer_getBytesUsed(src);
    if(count > bytes_used) {
        return -1;
    }

    uint8_t *u8dst = (uint8_t *)dst;
    const uint8_t *bufend = RingBuffer_getEnd(src);
    uint32_t nwritten = 0;

    while(nwritten != count) {
        uint32_t n = MIN(bufend - (src->buffer + src->base->tail), count - nwritten);
        memcpy(u8dst + nwritten, (src->buffer + src->base->tail), n);
        src->base->tail += n;
        nwritten += n;

        // Wrap?
        if((src->buffer + src->base->tail) == bufend) {
            src->base->tail = 0;
        }
    }

    return count;
}
