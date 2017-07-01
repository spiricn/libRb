/********************************************************/
/*                 Includes                             */
/********************************************************/
#include "rb/BufferQueue.h"
#include "rb/Utils.h"
#include "rb/BlockingQueue.h"
#include "rb/priv/ErrorPriv.h"

#include <pthread.h>

/********************************************************/
/*                 Defines                              */
/********************************************************/

#define BQ_CONTEXT_MAGIC ( 0x3435511 )
#define MAX_BUFFERS ( 64 )

#define BUFFER_LOCK do { pthread_mutex_lock(&bq->bufferMutex); }while(0)
#define BUFFER_UNLOCK do { pthread_mutex_unlock(&bq->bufferMutex); }while(0)

/********************************************************/
/*                 Typedefs                             */
/********************************************************/
typedef enum {
    eQUEUE_INVALID = -1, eQUEUE_FREE, eQUEUE_INPUT, eQUEUE_MAX
} Queue;

typedef struct {
    bool used;
    void* data;
    int32_t size;
    int32_t index;
    bool owned;
    bool queued;
} BufferContext;

typedef struct {
    int32_t magic;
    Rb_BlockingQueueHandle queues[eQUEUE_MAX];
    BufferContext buffers[MAX_BUFFERS];
    pthread_mutex_t bufferMutex;
} BufferQueueContext;

/********************************************************/
/*                 Local Functions Declarations (LOCAL) */
/********************************************************/

static BufferQueueContext* Rb_BufferQueuePriv_getContext(
        Rb_BufferQueueHandle handle);

static int32_t Rb_BufferQueuePriv_queueBuffer(BufferQueueContext* bq,
        Queue queue, int32_t index);

static int32_t Rb_BufferQueuePriv_dequeueBuffer(BufferQueueContext* bq,
        Queue queue, int32_t* index, int32_t timeoutMs);

static int32_t Rb_BufferQueuePriv_addBuffer(BufferQueueContext* bq,
        void* buffer, int32_t size, int32_t index, bool owned);

/********************************************************/
/*                 Functions Definitions (LOCAL/GLOBAL) */
/********************************************************/

Rb_BufferQueueHandle Rb_BufferQueue_new(int32_t numBuffers, int32_t bufferSize) {
    int32_t rc;
    int32_t i;
    BufferQueueContext* bq = (BufferQueueContext*) RB_CALLOC(
            sizeof(BufferQueueContext));

    bq->magic = BQ_CONTEXT_MAGIC;

    rc = pthread_mutex_init(&bq->bufferMutex, 0);
    if (rc != 0) {
        RB_ERR("pthread_mutex_init failed");
        return NULL;
    }

    BUFFER_LOCK
    ;

    for (i = eQUEUE_FREE; i < eQUEUE_MAX; i++) {
        bq->queues[i] = Rb_BlockingQueue_new(sizeof(int32_t), MAX_BUFFERS);
        if (bq->queues[i] == NULL) {
            RB_ERR("Rb_BlockingQueue_new failed");
            BUFFER_UNLOCK
            ;
            return NULL;
        }
    }

    if (numBuffers > 0 && bufferSize > 0) {
        // Crate our own buffers
        int32_t i;
        for (i = 0; i < numBuffers; i++) {
            uint8_t* data = RB_MALLOC(bufferSize);

            rc = Rb_BufferQueuePriv_addBuffer(bq, data, bufferSize,
            Rb_BUFFER_QUEUE_INVALID_INDEX /* generate an index */, true);
            if (rc != RB_OK) {
                RB_ERR("Rb_BufferQueuePriv_addBuffer failed");
                BUFFER_UNLOCK
                ;
                return NULL;
            }
        }
    }

    BUFFER_UNLOCK
    ;

    return (Rb_BufferQueueHandle) bq;
}

int32_t Rb_BufferQueue_free(Rb_BufferQueueHandle* handle) {
    int32_t rc;
    int32_t i;

    BufferQueueContext* bq = Rb_BufferQueuePriv_getContext(
            handle ? *handle : NULL);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    // Free only the buffers we own
    for (i = 0; i < MAX_BUFFERS; i++) {
        if (bq->buffers[i].used && bq->buffers[i].owned) {
            RB_FREE(&bq->buffers[i].data);
        }
    }

    for (i = eQUEUE_FREE; i < eQUEUE_MAX; i++) {
        rc = Rb_BlockingQueue_free(&bq->queues[i]);
        if (rc != RB_OK) {
            RB_ERR("Rb_BlockingQueue_free failed");
            BUFFER_UNLOCK
            ;
            return RB_ERROR;
        }
    }

    RB_FREE(&bq);
    *handle = NULL;

    return RB_OK;
}

int32_t Rb_BufferQueuePriv_addBuffer(BufferQueueContext* bq, void* buffer,
        int32_t size, int32_t index, bool owned) {
    int32_t rc;
    int32_t rrc;
    int32_t i;

    if (index == Rb_BUFFER_QUEUE_INVALID_INDEX) {
        for (i = 0; i < MAX_BUFFERS; i++) {
            if (!bq->buffers[i].used) {
                index = i;
                break;
            }
        }
    }

    if (index >= MAX_BUFFERS) {
        // TODO Resize buffers array ?
        RB_ERRC(RB_ERROR, "Max buffer number exceeded: %d", MAX_BUFFERS);
    }

    bq->buffers[index].used = true;
    bq->buffers[index].owned = owned;
    bq->buffers[index].data = buffer;
    bq->buffers[index].size = size;
    bq->buffers[index].index = index;
    bq->buffers[index].queued = false;

    rc = Rb_BufferQueuePriv_queueBuffer(bq, eQUEUE_FREE, index);
    if (rc != RB_OK) {
        RB_ERRC(RB_ERROR, "Rb_BufferQueuePriv_queueBuffer failed");
    }

    return RB_OK;
}

int32_t Rb_BufferQueue_addBuffer(Rb_BufferQueueHandle handle, void* buffer,
        int32_t size, int32_t index) {
    BufferQueueContext* bq = Rb_BufferQueuePriv_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    BUFFER_LOCK
    ;

    int32_t res = Rb_BufferQueuePriv_addBuffer(bq, buffer, size, index, false);

    BUFFER_UNLOCK
    ;

    return res;
}

int32_t Rb_BufferQueue_getBuffer(Rb_BufferQueueHandle handle, int32_t index,
        void** data, int32_t* size) {
    BufferQueueContext* bq = Rb_BufferQueuePriv_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    BUFFER_LOCK
    ;
    if (index < 0 || index >= MAX_BUFFERS) {
        BUFFER_UNLOCK
        ;
        RB_ERRC(RB_INVALID_ARG, "Invalid buffer index: %d", index);
    }

    else if (!bq->buffers[index].used) {
        BUFFER_UNLOCK
        ;
        RB_ERRC(RB_INVALID_ARG, "Buffer not added");

    }

    *data = bq->buffers[index].data;
    *size = bq->buffers[index].size;

    BUFFER_UNLOCK
    ;

    return RB_OK;
}

int32_t Rb_BufferQueue_containsBuffer(Rb_BufferQueueHandle handle,
        int32_t index) {
    BufferQueueContext* bq = Rb_BufferQueuePriv_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    BUFFER_LOCK
    ;
    bool res = bq->buffers[index].used;

    BUFFER_UNLOCK
    ;

    return res ? RB_TRUE : RB_FALSE;
}

int32_t Rb_BufferQueue_dequeueInputBuffer(Rb_BufferQueueHandle handle,
        int32_t* index, int32_t timeoutMs) {
    BufferQueueContext* bq = Rb_BufferQueuePriv_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return Rb_BufferQueuePriv_dequeueBuffer(bq, eQUEUE_INPUT, index, timeoutMs);
}

int32_t Rb_BufferQueue_queueInputBuffer(Rb_BufferQueueHandle handle,
        int32_t index) {
    BufferQueueContext* bq = Rb_BufferQueuePriv_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return Rb_BufferQueuePriv_queueBuffer(bq, eQUEUE_INPUT, index);
}

int32_t Rb_BufferQueuePriv_dequeueBuffer(BufferQueueContext* bq, Queue queue,
        int32_t* index, int32_t timeoutMs) {
    return Rb_BlockingQueue_get(bq->queues[queue], index);
}

int32_t Rb_BufferQueuePriv_queueBuffer(BufferQueueContext* bq, Queue queue,
        int32_t index) {
    return Rb_BlockingQueue_put(bq->queues[queue], &index);
}

int32_t Rb_BufferQueue_dequeueFreeBuffer(Rb_BufferQueueHandle handle,
        int32_t* index, int32_t timeoutMs) {
    BufferQueueContext* bq = Rb_BufferQueuePriv_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return Rb_BufferQueuePriv_dequeueBuffer(bq, eQUEUE_FREE, index, timeoutMs);
}

int32_t Rb_BufferQueue_queueFreeBuffer(Rb_BufferQueueHandle handle,
        int32_t index) {
    BufferQueueContext* bq = Rb_BufferQueuePriv_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return Rb_BufferQueuePriv_queueBuffer(bq, eQUEUE_FREE, index);
}

BufferQueueContext* Rb_BufferQueuePriv_getContext(Rb_BufferQueueHandle handle) {
    BufferQueueContext* bq = (BufferQueueContext*) handle;

    if (bq == NULL) {
        return NULL;
    }

    if (bq->magic != BQ_CONTEXT_MAGIC) {
        return NULL;
    }

    return bq;
}

