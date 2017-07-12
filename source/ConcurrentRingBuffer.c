/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/ConcurrentRingBuffer.h"
#include "rb/RingBuffer.h"
#include "rb/Utils.h"
#include "rb/ConsumerProducer.h"
#include "rb/priv/ErrorPriv.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define CONCURRENT_RING_BUFFER_MAGIC ( 0xC04C6B43 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    uint32_t magic;
    Rb_RingBufferHandle buffer;
    Rb_ConsumerProducerHandle cp;
} CRingBufferContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static CRingBufferContext* CRingBufferPriv_getContext(Rb_CRingBufferHandle handle);

static int32_t CRingBufferPriv_canWrite(void* arg);

static int32_t CRingBufferPriv_canRead(void* arg);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_CRingBufferHandle Rb_CRingBuffer_new(int32_t size) {
    if(size <= 0) {
        RB_ERR("Invalid size: %d", size);
        return NULL;
    }

    CRingBufferContext* rb = (CRingBufferContext*) RB_CALLOC(sizeof(CRingBufferContext));

    rb->magic = CONCURRENT_RING_BUFFER_MAGIC;

    rb->buffer = Rb_RingBuffer_new(size);
    if (rb->buffer == NULL) {
        RB_ERR("Rb_RingBuffer_new failed");
        return NULL;
    }

    rb->cp = Rb_ConsumerProducer_new();
    if (rb->cp == NULL) {
        RB_ERR("Rb_ConsumerProducer_new failed");
        return NULL;
    }

    return rb;
}

int32_t Rb_CRingBuffer_free(Rb_CRingBufferHandle* handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(*handle);
    if(rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    int32_t rc;

    rc = Rb_RingBuffer_free(&rb->buffer);
    if(rc != RB_OK){
        RB_ERRC(rc, "Rb_RingBuffer_free failed");
    }

    rc = Rb_ConsumerProducer_free(&rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_free failed");
    }

    RB_FREE(&rb);
    *handle = NULL;

    return RB_OK;
}

int32_t Rb_CRingBuffer_readTimed(Rb_CRingBufferHandle handle, uint8_t* data, int32_t size, Rb_CRingBuffer_ReadMode mode, int64_t timeoutMs, int32_t* bytesRead){
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if (rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    } else if (size <= 0) {
        RB_ERRC(RB_INVALID_ARG, "Invalid size: %d", size);
    }

    switch (mode) {
    case eCRB_READ_MODE_ALL:
    case eCRB_READ_MODE_SOME:
        break;
    default:
        RB_ERRC(RB_INVALID_ARG, "Invalid read mode: %d", mode);
    }

    *bytesRead = 0;

    int32_t bytesRemaining = size;

    while(true){
        rc = Rb_ConsumerProducer_acquireReadLock(rb->cp,
                CRingBufferPriv_canRead, rb, timeoutMs);
        if (rc == RB_TIMEOUT || rc == RB_DISABLED) {
            *bytesRead = size - bytesRemaining;
            return rc;
        } else if (rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_acquireReadLock failed");
        }

        int32_t bytesAvailable = Rb_RingBuffer_getBytesUsed(rb->buffer);
        if (bytesAvailable <= 0) {
            RB_ERRC(RB_ERROR, "Rb_RingBuffer_getBytesUsed failed: %d", bytesAvailable);
        }

        int32_t bytesToRead = bytesAvailable < bytesRemaining ? bytesAvailable : bytesRemaining;

        bytesRemaining -= bytesToRead;

        rc = Rb_ConsumerProducer_releaseReadLock(rb->cp, true);
        if (rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_releaseReadLock failed");
        }

        if(mode == eCRB_READ_MODE_SOME || (mode == eCRB_READ_MODE_ALL && bytesRemaining == 0)){
            *bytesRead = size - bytesRemaining;
            return RB_OK;
        }
    }
}

int32_t Rb_CRingBuffer_read(Rb_CRingBufferHandle handle, uint8_t* data, int32_t size,
        Rb_CRingBuffer_ReadMode mode, int32_t* bytesRead) {
    return Rb_CRingBuffer_readTimed(handle, data, size, mode, RB_WAIT_INFINITE, bytesRead);
}

int32_t Rb_CRingBuffer_writeTimed(Rb_CRingBufferHandle handle, const uint8_t* data,
        int32_t size, Rb_CRingBuffer_WriteMode mode, int64_t timeoutMs, int32_t* bytesWritten){
    int32_t rc;
    int32_t res;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }
    else if (size <= 0) {
        RB_ERRC(RB_INVALID_ARG, "Invalid size: %d", size);
    }

    switch (mode) {
    case eCRB_WRITE_MODE_ALL:
    case eCRB_WRITE_MODE_OVERFLOW:
    case eCRB_WRITE_MODE_SOME:
        break;
    default:
        RB_ERRC(RB_INVALID_ARG, "Invalid write mode: %d", mode);
    }

    *bytesWritten = 0;

    if(mode ==  eCRB_WRITE_MODE_OVERFLOW){
        bool enabled = false;

        rc = Rb_ConsumerProducer_acquireLock(rb->cp, &enabled);
        if (rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
        }

        // We acquired the global lock, but our CP is disable so just return
        if(!enabled){
            rc = Rb_ConsumerProducer_releaseLock(rb->cp);
            if (rc != RB_OK) {
                RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
            }

            return RB_DISABLED;
        }

        rc = Rb_RingBuffer_write(rb->buffer, data, size);
        if (rc != size) {
            RB_ERRC(rc, "Rb_RingBuffer_write failed");
        }

        rc = Rb_ConsumerProducer_notifyWritten(rb->cp);
        if (rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_notifyWritten failed");
        }

        rc = Rb_ConsumerProducer_releaseLock(rb->cp);
        if (rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
        }

        *bytesWritten = size;
        return RB_OK;
    }
    else if(mode == eCRB_WRITE_MODE_SOME) {
        // Acquire write lock
        rc = Rb_ConsumerProducer_acquireWriteLock(rb->cp,
                CRingBufferPriv_canWrite, rb, timeoutMs);
        if (rc == RB_TIMEOUT || rc == RB_DISABLED) {
            return rc;
        } else if (rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
        }

        int32_t bytesFree = Rb_RingBuffer_getBytesFree(rb->buffer);
        if (bytesFree <= 0) {
            RB_ERRC(RB_ERROR, "Rb_RingBuffer_getBytesFree failed");
        }

        int32_t bytesToWrite =
                bytesFree < size ? bytesFree : size;

        rc = Rb_RingBuffer_write(rb->buffer, data, bytesToWrite);
        if (rc != bytesToWrite) {
            RB_ERRC(RB_ERROR, "Rb_RingBuffer_write failed");
        }

        // Release write lock
        rc = Rb_ConsumerProducer_releaseWriteLock(rb->cp, true);
        if (rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_releaseWriteLock failed");
        }

        *bytesWritten = bytesToWrite;
        return RB_OK;
    }
    else if(mode == eCRB_WRITE_MODE_ALL){
        int32_t bytesRemaining = size;

        while(true) {
            // Acquire write lock
            rc = Rb_ConsumerProducer_acquireWriteLock(rb->cp,
                    CRingBufferPriv_canWrite, rb, timeoutMs);
            if (rc == RB_TIMEOUT || rc == RB_DISABLED) {
                *bytesWritten = size - bytesRemaining;
                return rc;
            } else if (rc != RB_OK) {
                RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
            }

            int32_t bytesFree = Rb_RingBuffer_getBytesFree(rb->buffer);
            if (bytesFree <= 0) {
                RB_ERRC(RB_ERROR, "Rb_RingBuffer_getBytesFree failed");
            }

            int32_t bytesToWrite = bytesFree < bytesRemaining ? bytesFree : bytesRemaining;

            rc = Rb_RingBuffer_write(rb->buffer, &data[size - bytesRemaining], bytesToWrite);
            if (rc != bytesToWrite) {
                RB_ERRC(RB_ERROR, "Rb_RingBuffer_write failed");
            }
            bytesRemaining -= bytesToWrite;

            // Release write lock
            rc = Rb_ConsumerProducer_releaseWriteLock(rb->cp, true);
            if (rc != RB_OK) {
                RB_ERRC(rc, "Rb_ConsumerProducer_releaseWriteLock failed");
            }

            if (bytesRemaining == 0) {
                *bytesWritten = size;
                return RB_OK;
            }
        }
    }
    else {
        // Sanity check
        RB_ERRC(RB_ERROR, "Invalid mode: %d", mode);
    }
}

int32_t Rb_CRingBuffer_write(Rb_CRingBufferHandle handle, const uint8_t* data,
        int32_t size, Rb_CRingBuffer_WriteMode mode, int32_t* bytesWritten) {
    return Rb_CRingBuffer_writeTimed(handle, data, size, mode, RB_WAIT_INFINITE, bytesWritten);
}

int32_t Rb_CRingBuffer_getBytesUsed(Rb_CRingBufferHandle handle) {
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if (rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(rb->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_RingBuffer_getBytesUsed(rb->buffer);

    rc = Rb_ConsumerProducer_releaseLock(rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return res;
}

int32_t Rb_CRingBuffer_getBytesFree(Rb_CRingBufferHandle handle) {
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(rb->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_RingBuffer_getBytesFree(rb->buffer);

    rc = Rb_ConsumerProducer_releaseLock(rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return res;
}

int32_t Rb_CRingBuffer_getCapacity(Rb_CRingBufferHandle handle) {
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if (rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(rb->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_RingBuffer_getCapacity(rb->buffer);

    rc = Rb_ConsumerProducer_releaseLock(rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return res;
}

int32_t Rb_CRingBuffer_disable(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if (rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return Rb_ConsumerProducer_enable(rb->cp);
}

int32_t Rb_CRingBuffer_enable(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if (rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return Rb_ConsumerProducer_enable(rb->cp);
}

int32_t Rb_CRingBuffer_isEnabled(Rb_CRingBufferHandle handle) {
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if (rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(rb->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_ConsumerProducer_isEnabled(rb->cp);

    rc = Rb_ConsumerProducer_releaseLock(rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return res;
}

int32_t Rb_CRingBuffer_clear(Rb_CRingBufferHandle handle) {
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if (rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(rb->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    rc = Rb_RingBuffer_clear(rb->buffer);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_RingBuffer_clear failed");
    }

    rc = Rb_ConsumerProducer_notifyRead(rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_notifyRead failed");
    }

    rc = Rb_ConsumerProducer_releaseLock(rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    return RB_OK;
}

int32_t Rb_CRingBuffer_isFull(Rb_CRingBufferHandle handle) {
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(rb->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_RingBuffer_isFull(rb->buffer);

    rc = Rb_ConsumerProducer_releaseLock(rb->cp);
    if(rc != RB_OK){
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return res;
}

int32_t Rb_CRingBuffer_isEmpty(Rb_CRingBufferHandle handle) {
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(rb->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_RingBuffer_isEmpty(rb->buffer);

    rc = Rb_ConsumerProducer_releaseLock(rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return res;
}

int32_t Rb_CRingBuffer_resize(Rb_CRingBufferHandle handle, uint32_t capacity){
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(rb->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t res = Rb_RingBuffer_resize(rb->buffer, capacity);

    rc = Rb_ConsumerProducer_releaseLock(rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return res;
}

int32_t CRingBufferPriv_canWrite(void* arg) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(arg);
    if (rb == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t bytesFree = Rb_RingBuffer_getBytesFree(rb->buffer);
    if (bytesFree < 0) {
        return bytesFree;
    }

    return bytesFree > 0 ? RB_TRUE : RB_FALSE;
}

int32_t CRingBufferPriv_canRead(void* arg) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(arg);
    if (rb == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t bytesUsed = Rb_RingBuffer_getBytesUsed(rb->buffer);
    if (bytesUsed < 0) {
        return bytesUsed;
    }

    return bytesUsed > 0 ? RB_TRUE : RB_FALSE;
}

CRingBufferContext* CRingBufferPriv_getContext(Rb_CRingBufferHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    CRingBufferContext* rb = (CRingBufferContext*) handle;
    if(rb->magic != CONCURRENT_RING_BUFFER_MAGIC) {
        return NULL;
    }

    return rb;
}

float Rb_CRingBuffer_usedSpacePercentage(Rb_CRingBufferHandle handle) {
    int32_t rc;

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(rb->cp, NULL);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    int32_t bytesUsed = Rb_RingBuffer_getBytesUsed(rb->buffer);
    int32_t capacity = Rb_RingBuffer_getCapacity(rb->buffer);

    float res = 100 * (float) bytesUsed / (float) capacity;

    rc = Rb_ConsumerProducer_releaseLock(rb->cp);
    if (rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return res;
}
