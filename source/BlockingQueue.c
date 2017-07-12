/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/BlockingQueue.h"
#include "rb/ConsumerProducer.h"
#include "rb/Common.h"
#include "rb/priv/ErrorPriv.h"
#include "rb/Utils.h"
#include "rb/List.h"
#include "rb/Log.h"

#include <stdio.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define BLOCKING_QUEUE_MAGIC ( 0x3333ABFF )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    int32_t magic;
    Rb_ListHandle list;
    Rb_ConsumerProducerHandle cp;
    int32_t capacity;
} BlockingQueueContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static BlockingQueueContext* BlockingQueue_getContext(
        Rb_BlockingQueueHandle handle);

static int32_t BlockingQueuePriv_get(BlockingQueueContext* bq, void* message,
        bool pop, int64_t timeoutMs);

static int32_t BlockingQueuePriv_put(BlockingQueueContext* bq, const void* message, int64_t timeoutMs);

static int32_t BlockingQueuePriv_canWrite(void *arg);

static int32_t BlockingQueuePriv_canRead(void *arg);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int32_t BlockingQueuePriv_canWrite(void *arg) {
    BlockingQueueContext* bq = BlockingQueue_getContext(arg);
    if (bq == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t size = Rb_List_getSize(bq->list);
    if (size < 0) {
        return RB_ERROR;
    }

    return size < bq->capacity ? RB_TRUE : RB_FALSE;
}

int32_t BlockingQueuePriv_canRead(void *arg) {
    BlockingQueueContext* bq = BlockingQueue_getContext(arg);
    if (bq == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t size = Rb_List_getSize(bq->list);
    if (size < 0) {
        return RB_ERROR;
    }

    return size > 0 ? RB_TRUE : RB_FALSE;
}

Rb_BlockingQueueHandle Rb_BlockingQueue_new(int32_t messageSize,
        int32_t capacity) {
    if (messageSize <= 0 || capacity <= 0) {
        RB_ERR("Invalid arguments");
        return NULL;
    }

    BlockingQueueContext* bq = RB_CALLOC(sizeof(BlockingQueueContext));

    bq->magic = BLOCKING_QUEUE_MAGIC;
    bq->list = Rb_List_new(messageSize);
    bq->cp = Rb_ConsumerProducer_new();
    bq->capacity = capacity;

    return (Rb_BlockingQueueHandle) bq;
}

int32_t Rb_BlockingQueue_free(Rb_BlockingQueueHandle* handle) {
    BlockingQueueContext* bq = BlockingQueue_getContext(
            handle ? *handle : NULL);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    int32_t rc;

    rc = Rb_List_free(&bq->list);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_List_free failed");
    }

    rc = Rb_ConsumerProducer_free(&bq->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_free failed");
    }

    RB_FREE(&bq);
    *handle = NULL;

    return RB_OK;
}

int32_t Rb_BlockingQueue_put(Rb_BlockingQueueHandle handle, const void* message) {
    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }
    return BlockingQueuePriv_put(bq, message, RB_WAIT_INFINITE);
}

int32_t Rb_BlockingQueue_peek(Rb_BlockingQueueHandle handle, void* message) {
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return BlockingQueuePriv_get(bq, message, false, RB_WAIT_INFINITE);
}

int32_t Rb_BlockingQueue_get(Rb_BlockingQueueHandle handle, void* message) {
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return BlockingQueuePriv_get(bq, message, true, RB_WAIT_INFINITE);
}

int32_t Rb_BlockingQueue_isFull(Rb_BlockingQueueHandle handle) {
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(bq->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res =
            Rb_List_getSize(bq->list) == bq->capacity ? RB_TRUE : RB_FALSE;

    rc = Rb_ConsumerProducer_releaseLock(bq->cp);
    if (rc != RB_OK) {
        return rc;
    }

    return res;
}

int32_t Rb_BlockingQueue_isEmpty(Rb_BlockingQueueHandle handle) {
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(bq->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_List_getSize(bq->list) == 0 ? RB_TRUE : RB_FALSE;

    rc = Rb_ConsumerProducer_releaseLock(bq->cp);
    if (rc != RB_OK) {
        return rc;
    }

    return res;
}

int32_t Rb_BlockingQueue_resize(Rb_BlockingQueueHandle handle, int32_t capacity) {
    int32_t rc;
    int32_t i;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    } else if (capacity <= 0) {
        RB_ERRC(RB_INVALID_ARG, "Invalid capacity");
    }

    rc = Rb_ConsumerProducer_acquireLock(bq->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_List_getSize(bq->list);

    // Notify there's free space only when we're at max capactiy and we increase the it
    bool notifyRead = res == bq->capacity && capacity > bq->capacity;

    if (capacity < res) {
        // Remove excess elements
        for (i = 0; i < res - capacity; i++) {
            rc = Rb_List_remove(bq->list, 0);
            if (rc != RB_OK) {
                RB_ERRC(rc, "Rb_List_remove failed");
            }
        }
    }

    bq->capacity = capacity;

    if (notifyRead) {
        rc = Rb_ConsumerProducer_notifyRead(bq->cp);
        if (rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_notifyRead failed");
        }
    }

    rc = Rb_ConsumerProducer_releaseLock(bq->cp);
    if (rc != RB_OK) {
        return rc;
    }

    return RB_OK;
}

int32_t Rb_BlockingQueue_clear(Rb_BlockingQueueHandle handle) {
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(bq->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_List_clear(bq->list);

    rc = Rb_ConsumerProducer_notifyRead(bq->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_notifyRead failed");
    }

    rc = Rb_ConsumerProducer_releaseLock(bq->cp);
    if (rc != RB_OK) {
        return rc;
    }

    return res;
}

int32_t Rb_BlockingQueue_getNumMessages(Rb_BlockingQueueHandle handle) {
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(bq->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_List_getSize(bq->list);

    rc = Rb_ConsumerProducer_releaseLock(bq->cp);
    if (rc != RB_OK) {
        return rc;
    }

    return res;
}

int32_t Rb_BlockingQueue_getCapacity(Rb_BlockingQueueHandle handle) {
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(bq->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = bq->capacity;

    rc = Rb_ConsumerProducer_releaseLock(bq->cp);
    if (rc != RB_OK) {
        return rc;
    }

    return res;
}

int32_t Rb_BlockingQueue_disable(Rb_BlockingQueueHandle handle) {
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return Rb_ConsumerProducer_disable(bq->cp);
}

int32_t Rb_BlockingQueue_enable(Rb_BlockingQueueHandle handle) {
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return Rb_ConsumerProducer_enable(bq->cp);
}

int32_t Rb_BlockingQueue_isEnabled(Rb_BlockingQueueHandle handle){
    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return Rb_ConsumerProducer_isEnabled(bq->cp);
}

int32_t Rb_BlockingQueue_putTimed(Rb_BlockingQueueHandle handle,
        const void* message, int64_t timeoutMs){
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return BlockingQueuePriv_put(bq, message, timeoutMs);
}

int32_t Rb_BlockingQueue_getTimed(Rb_BlockingQueueHandle handle, void* message, int64_t timeoutMs){
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return BlockingQueuePriv_get(bq, message, true, timeoutMs);
}

int32_t Rb_BlockingQueue_peekTimed(Rb_BlockingQueueHandle handle, void* message, int64_t timeoutMs){
    int32_t rc;

    BlockingQueueContext* bq = BlockingQueue_getContext(handle);
    if (bq == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return BlockingQueuePriv_get(bq, message, false, timeoutMs);
}

int32_t BlockingQueuePriv_put(BlockingQueueContext* bq, const void* message, int64_t timeoutMs){
    int32_t rc;

    if (timeoutMs != RB_WAIT_INFINITE && timeoutMs <= 0) {
        RB_ERRC(RB_INVALID_ARG, "Invalid timeout value: %lld", timeoutMs);
    } else if (message == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid message arg: %p", message);
    }

    rc = Rb_ConsumerProducer_acquireWriteLock(bq->cp,
            BlockingQueuePriv_canWrite, bq, timeoutMs);
    if(rc == RB_TIMEOUT || rc == RB_DISABLED){
        return rc;
    }
    else if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireWriteLock failed");
    }

    rc = Rb_List_insert(bq->list, 0, message);
    if (rc != RB_OK) {
        RB_ERR("Rb_List_add failed ( %s )", Rb_getLastErrorMessage());
        return rc;
    }

    rc = Rb_ConsumerProducer_releaseWriteLock(bq->cp, true);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseWriteLock failed");
    }

    return RB_OK;
}

int32_t BlockingQueuePriv_get(BlockingQueueContext* bq, void* message, bool pop, int64_t timeoutMs) {
    int32_t rc;

    if (timeoutMs != RB_WAIT_INFINITE && timeoutMs <= 0) {
        RB_ERRC(RB_INVALID_ARG, "Invalid timeout value: %lld", timeoutMs);
    } else if (message == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid message arg: %p", message);
    }

    rc = Rb_ConsumerProducer_acquireReadLock(bq->cp, BlockingQueuePriv_canRead,
            bq, timeoutMs);
    if(rc == RB_TIMEOUT || rc == RB_DISABLED){
        return rc;
    }
    else if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireReadLock failed:\n%s", Rb_getLastErrorMessage());
    }

    int32_t size = Rb_List_getSize(bq->list);

    int32_t index = size - 1;

    rc = Rb_List_get(bq->list, index, message);
    if (rc != RB_OK) {
        RB_ERR("Rb_List_get failed ( %s )", Rb_getLastErrorMessage());
        // TODO
        return rc;
    }

    if (pop) {
        rc = Rb_List_remove(bq->list, index);
        if (rc != RB_OK) {
            RB_ERR("Rb_List_remove failed ( %s )", Rb_getLastErrorMessage());
            // TODO
            return rc;
        }
    }

    rc = Rb_ConsumerProducer_releaseReadLock(bq->cp, pop);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseReadLock failed");
    }

    return RB_OK;
}

BlockingQueueContext* BlockingQueue_getContext(Rb_BlockingQueueHandle handle) {
    if (handle == NULL) {
        return NULL;
    }

    BlockingQueueContext* bq = (BlockingQueueContext*) handle;
    if (bq->magic != BLOCKING_QUEUE_MAGIC) {
        return NULL;
    }

    return bq;
}
