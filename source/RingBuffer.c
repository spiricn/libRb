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

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t size;
} RingBufferBase;

typedef struct {
    uint32_t magic;
    uint8_t* buffer;
    RingBufferBase* base;
    int sharedMemory;
} RingBufferContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static const uint8_t* RingBufferPriv_getEnd(RingBufferHandle handle);

static uint8_t* RingBufferPriv_nextp(RingBufferHandle handle, const uint8_t *p);

static RingBufferContext* RingBufferPriv_getContext(RingBufferHandle handle);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

RingBufferHandle RingBuffer_fromSharedMemory(void* vptr, uint32_t size,
        int init) {
    RingBufferContext* rb = (RingBufferContext*) malloc(
            sizeof(RingBufferContext));
    memset(rb, 0x00, sizeof(RingBufferContext));

    uint8_t* data = (uint8_t*) vptr;

    rb->base = (RingBufferBase*) data;
    rb->base->size = size - sizeof(RingBufferBase);
    rb->buffer = data + sizeof(RingBufferBase);
    rb->magic = RING_BUFFER_MAGIC;
    rb->sharedMemory = 1;

    if(init) {
        RingBuffer_clear(rb);
    }

    return (RingBufferHandle) rb;
}

RingBufferHandle RingBuffer_new(uint32_t capacity) {
    RingBufferContext* rb = (RingBufferContext*) calloc(1,
            sizeof(RingBufferContext));

    rb->base = (RingBufferBase*) calloc(1, sizeof(RingBufferBase));
    rb->base->size = capacity + 1 /* One byte is used for detecting the full condition. */;
    rb->buffer = (uint8_t*) malloc(rb->base->size);
    rb->magic = RING_BUFFER_MAGIC;
    rb->sharedMemory = 0;

    RingBuffer_clear(rb);

    return (RingBufferHandle) rb;
}

int32_t RingBuffer_free(RingBufferHandle* handle) {
    RingBufferContext* rb = RingBufferPriv_getContext(*handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    if(!rb->sharedMemory) {
        free(rb->buffer);
        free(rb->base);
    }

    free(rb);
    *handle = NULL;

    return RB_OK;
}

int32_t RingBuffer_getSize(RingBufferHandle handle) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    return rb->base->size;
}

int32_t RingBuffer_clear(RingBufferHandle handle) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return -1;
    }

    rb->base->head = rb->base->tail = 0;

    return 0;
}

int32_t RingBuffer_getCapacity(RingBufferHandle handle) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return -1;
    }

    return RingBuffer_getSize(rb) - 1;
}

const uint8_t* RingBufferPriv_getEnd(RingBufferHandle handle) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return NULL;
    }

    return rb->buffer + RingBuffer_getSize(rb);
}

int32_t RingBuffer_getBytesFree(RingBufferHandle handle) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    if(rb->base->head >= rb->base->tail) {
        return RingBuffer_getCapacity(rb) - (rb->base->head - rb->base->tail);
    } else {
        return rb->base->tail - rb->base->head - 1;
    }
}

int32_t RingBuffer_getBytesUsed(RingBufferHandle handle) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    return RingBuffer_getCapacity(rb) - RingBuffer_getBytesFree(rb);
}

int32_t RingBuffer_isFull(RingBufferHandle handle) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    return RingBuffer_getBytesFree(rb) == 0;
}

int32_t RingBuffer_isEmpty(RingBufferHandle handle) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    return RingBuffer_getBytesFree(rb) == RingBuffer_getCapacity(rb);
}

uint8_t* RingBufferPriv_nextp(RingBufferHandle handle, const uint8_t *p) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return NULL;
    }

    return rb->buffer + ((++p - rb->buffer) % RingBuffer_getSize(rb));
}

int32_t RingBuffer_write(RingBufferHandle handle, const void *src,
        uint32_t count) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    const uint8_t* u8src = (const uint8_t *) src;
    const uint8_t* bufend = RingBufferPriv_getEnd(rb);
    int overflow = (int32_t) count > RingBuffer_getBytesFree(rb);
    uint32_t nread = 0;

    while(nread != count) {
        // Don't copy beyond the end of the buffer
        uint32_t n = MIN(bufend - (rb->buffer + rb->base->head),
                (int32_t)(count - nread));
        memcpy((rb->buffer + rb->base->head), u8src + nread, n);
        rb->base->head += n;
        nread += n;

        // Wrap ?
        if((rb->buffer + rb->base->head) == bufend) {
            rb->base->head = 0;
        }
    }
    if(overflow) {
        rb->base->tail = RingBufferPriv_nextp(rb, (rb->buffer + rb->base->head))
                - rb->buffer;
    }

    return count;
}

int32_t RingBuffer_read(RingBufferHandle handle, void *dst, uint32_t count) {
    RingBufferContext* rb = RingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    uint32_t bytes_used = RingBuffer_getBytesUsed(rb);
    if(count > bytes_used) {
        return RB_ERROR;
    }

    uint8_t *u8dst = (uint8_t *) dst;
    const uint8_t *bufend = RingBufferPriv_getEnd(rb);
    uint32_t nwritten = 0;

    while(nwritten != count) {
        uint32_t n = MIN(bufend - (rb->buffer + rb->base->tail),
                (int32_t)(count - nwritten));
        memcpy(u8dst + nwritten, (rb->buffer + rb->base->tail), n);
        rb->base->tail += n;
        nwritten += n;

        // Wrap?
        if((rb->buffer + rb->base->tail) == bufend) {
            rb->base->tail = 0;
        }
    }

    return count;
}

int32_t RingBuffer_resize(RingBufferHandle handle, uint32_t capacity) {
	RingBufferContext* rb = RingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return RB_INVALID_ARG;
	}

	if (rb->sharedMemory) {
		// Not yet implemented
		return RB_INVALID_ARG;
	}

	if (capacity < rb->base->size) {
		// Shrinking not yet implemented
		return RB_INVALID_ARG;
	}

	rb->base->size = capacity + 1;
	rb->buffer = (uint8_t*) realloc(rb->buffer, rb->base->size);

	return RB_OK;
}

RingBufferContext* RingBufferPriv_getContext(RingBufferHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    RingBufferContext* rb = (RingBufferContext*) handle;
    if(rb->magic != RING_BUFFER_MAGIC) {
        return NULL;
    }

    return rb;
}
