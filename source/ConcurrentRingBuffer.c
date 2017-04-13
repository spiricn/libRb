/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/ConcurrentRingBuffer.h"
#include "rb/RingBuffer.h"
#include "rb/Stopwatch.h"
#include "rb/Utils.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define CONCURRENT_RING_BUFFER_MAGIC ( 0xC04C6B43 )

#define LOCK_ACQUIRE do{ pthread_mutex_lock(&rb->base->mutex); }while(0)

#define LOCK_RELEASE do{ pthread_mutex_unlock(&rb->base->mutex); }while(0)

#define READ_LOCK do{ pthread_mutex_lock(&rb->base->readMutex); }while(0)

#define READ_RELEASE do{ pthread_mutex_unlock(&rb->base->readMutex); }while(0)

#define WRITE_LOCK do{ pthread_mutex_lock(&rb->base->writeMutex); }while(0)

#define WRITE_RELEASE do{ pthread_mutex_unlock(&rb->base->writeMutex); }while(0)

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    pthread_mutex_t mutex;
    pthread_mutex_t readMutex;
    pthread_mutex_t writeMutex;
    pthread_cond_t readCV;
    pthread_cond_t writeCV;
    int enabled;
} CRingBufferBase;

typedef struct {
    uint32_t magic;
    CRingBufferBase* base;
    Rb_RingBufferHandle buffer;
    int sharedMemory;
    int owned;
} CRingBufferContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static CRingBufferContext* CRingBufferPriv_getContext(Rb_CRingBufferHandle handle);

static bool CRingBufferPriv_timedLock(pthread_mutex_t* mutex, int64_t ms){
    if(ms == RB_WAIT_INFINITE){
        pthread_mutex_lock(mutex);

        return true;
    }
    else{
        if(ms <= 0){
            return false;
        }

        struct timespec time;
        Rb_Utils_getOffsetTime(&time, ms);

        return pthread_mutex_timedlock(mutex, &time) == 0;
    }
}

static bool CRingBufferPriv_timedWait(pthread_cond_t* cv, pthread_mutex_t* mutex, int64_t ms){
    if(ms == RB_WAIT_INFINITE){
        pthread_cond_wait(cv, mutex);

        return true;
    }
    else{
        if(ms <= 0){
            return false;
        }

        struct timespec time;
        Rb_Utils_getOffsetTime(&time, ms);

        return pthread_cond_timedwait(cv, mutex, &time) == 0;
    }
}

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_CRingBufferHandle Rb_CRingBuffer_fromSharedMemory(void* memory, uint32_t size,
        int init) {
    if(size == 0) {
        return NULL;
    }

    CRingBufferContext* rb = (CRingBufferContext*) RB_CALLOC(sizeof(CRingBufferContext));
    rb->base = (CRingBufferBase*) memory;

    rb->magic = CONCURRENT_RING_BUFFER_MAGIC;

    rb->buffer = Rb_RingBuffer_fromSharedMemory(
            ((uint8_t*) memory) + sizeof(CRingBufferBase),
            size - sizeof(CRingBufferBase), init);

    if(init) {
        rb->base->enabled = 1;

        pthread_mutexattr_t mutexAttr;
        pthread_mutexattr_init(&mutexAttr);
        pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&rb->base->mutex, &mutexAttr);

        pthread_condattr_t condAttr;
        pthread_condattr_init(&condAttr);
        pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);

        pthread_cond_init(&rb->base->readCV, &condAttr);

        pthread_cond_init(&rb->base->writeCV, &condAttr);

        rb->owned = 1;
    } else {
        rb->owned = 0;
    }

    rb->sharedMemory = 1;

    return rb;
}

Rb_CRingBufferHandle Rb_CRingBuffer_new(uint32_t size) {
    if(size == 0) {
        return NULL;
    }

    CRingBufferContext* rb = (CRingBufferContext*) RB_CALLOC(sizeof(CRingBufferContext));

    rb->base = (CRingBufferBase*) RB_MALLOC(sizeof(CRingBufferBase));
    rb->magic = CONCURRENT_RING_BUFFER_MAGIC;

    pthread_mutex_init(&rb->base->mutex, NULL);
    pthread_mutex_init(&rb->base->readMutex, NULL);
    pthread_mutex_init(&rb->base->writeMutex, NULL);
    pthread_cond_init(&rb->base->readCV, NULL);
    pthread_cond_init(&rb->base->writeCV, NULL);

    rb->buffer = Rb_RingBuffer_new(size);
    rb->base->enabled = 1;
    rb->sharedMemory = 0;
    rb->owned = 1;

    return rb;
}

int32_t Rb_CRingBuffer_free(Rb_CRingBufferHandle* handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(*handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    if(rb->owned) {
        pthread_mutex_destroy(&rb->base->mutex);
        pthread_mutex_destroy(&rb->base->writeMutex);
        pthread_mutex_destroy(&rb->base->readMutex);
        pthread_cond_destroy(&rb->base->readCV);
        pthread_cond_destroy(&rb->base->writeCV);
    }

    const int32_t res = Rb_RingBuffer_free(&rb->buffer);

    if(!rb->sharedMemory) {
        RB_FREE(&rb->base);
    }

    RB_FREE(&rb);
    *handle = NULL;

    return res;
}

int32_t Rb_CRingBuffer_readTimed(Rb_CRingBufferHandle handle, uint8_t* data, uint32_t size, Rb_CRingBuffer_ReadMode mode, int64_t timeoutMs){
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    Rb_StopwatchHandle sw = Rb_Stopwatch_new();
    Rb_Stopwatch_start(sw);

    uint32_t bytesRead = 0;

    // Checkpoint
    if(!rb->base->enabled) {
        Rb_Stopwatch_free(&sw);
        return 0;
    }

    // Read lock
    if(!CRingBufferPriv_timedLock(&rb->base->readMutex, timeoutMs)){
        Rb_Stopwatch_free(&sw);
        return RB_TIMEOUT;
    }

    // Buffer lock
    if(!CRingBufferPriv_timedLock(&rb->base->mutex, timeoutMs == RB_WAIT_INFINITE ? RB_WAIT_INFINITE : timeoutMs - Rb_Stopwatch_elapsedMs(&sw))){
        Rb_Stopwatch_free(&sw);
        return RB_TIMEOUT;
    }

    // Checkpoint
    if(!rb->base->enabled) {
        LOCK_RELEASE
        ;
        READ_RELEASE
        ;
        Rb_Stopwatch_free(&sw);
        return 0;
    }

    if(mode == eRB_READ_BLOCK_FULL) {
        uint32_t bytesRemaining = size;
        uint32_t bytesUsed = 0;

        while(bytesRemaining) {
            // Wait until some data is available
            while((bytesUsed = Rb_RingBuffer_getBytesUsed(rb->buffer)) == 0 && rb->base->enabled) {
                if(!CRingBufferPriv_timedWait(&rb->base->writeCV, &rb->base->mutex, timeoutMs == RB_WAIT_INFINITE ? RB_WAIT_INFINITE : timeoutMs - Rb_Stopwatch_elapsedMs(&sw))){
                    LOCK_RELEASE
                    ;
                    READ_RELEASE
                    ;
                    Rb_Stopwatch_free(&sw);
                    return RB_TIMEOUT;
                }
            }

            // Checkpoint
            if(!rb->base->enabled) {
                LOCK_RELEASE
                ;
                READ_RELEASE
                ;
                Rb_Stopwatch_free(&sw);
                return size - bytesRemaining;
            }

            const int32_t toRead = bytesRemaining < bytesUsed ? bytesRemaining : bytesUsed;

            Rb_RingBuffer_read(rb->buffer, data + (size - bytesRemaining), toRead);

            bytesRemaining -= toRead;

            pthread_cond_broadcast(&rb->base->readCV);
        }

        bytesRead = size - bytesRemaining;
    } else {
        if(mode == eRB_READ_BLOCK_PARTIAL) {
            // Wait at least some of the data we requires is available
            while(Rb_RingBuffer_getBytesUsed(rb->buffer) == 0 && rb->base->enabled) {
                if(!CRingBufferPriv_timedWait(&rb->base->writeCV, &rb->base->mutex, timeoutMs == RB_WAIT_INFINITE ? RB_WAIT_INFINITE : timeoutMs - Rb_Stopwatch_elapsedMs(&sw))){
                    LOCK_RELEASE
                    ;
                    READ_RELEASE
                    ;
                    Rb_Stopwatch_free(&sw);
                    return RB_TIMEOUT;
                }
            }

            // Checkpoint
            if(!rb->base->enabled) {
                LOCK_RELEASE
                ;
                READ_RELEASE
                ;
                Rb_Stopwatch_free(&sw);
                return 0;
            }

            uint32_t available = Rb_RingBuffer_getBytesUsed(rb->buffer);

            size = size > available ? available : size;
        } else if(mode == eRB_READ_BLOCK_NONE) {
            // Read whatever data is available at the moment (may be nothing)
            uint32_t available = Rb_RingBuffer_getBytesUsed(rb->buffer);

            size = size > available ? available : size;
        }

        if(size) {
            bytesRead = Rb_RingBuffer_read(rb->buffer, data, size);

            pthread_cond_broadcast(&rb->base->readCV);
        }
    }

    LOCK_RELEASE
    ;
    READ_RELEASE
    ;
    Rb_Stopwatch_free(&sw);
    return bytesRead;
}

int32_t Rb_CRingBuffer_read(Rb_CRingBufferHandle handle, uint8_t* data, uint32_t size,
        Rb_CRingBuffer_ReadMode mode) {
    return Rb_CRingBuffer_readTimed(handle, data, size, mode, RB_WAIT_INFINITE);
}

int32_t Rb_CRingBuffer_writeTimed(Rb_CRingBufferHandle handle, const uint8_t* data,
        uint32_t size, Rb_CRingBuffer_WriteMode mode, int64_t timeoutMs){

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    Rb_StopwatchHandle sw = Rb_Stopwatch_new();
    Rb_Stopwatch_start(&sw);

    // Total bytes written
    uint32_t bytesWritten = 0;

    // Checkpoint
    if(!rb->base->enabled) {
        Rb_Stopwatch_free(&sw);
        return 0;
    }

    // Write lock
    if(!CRingBufferPriv_timedLock(&rb->base->writeMutex, timeoutMs)){
        Rb_Stopwatch_free(&sw);
        return RB_TIMEOUT;
    }

    // Buffer lock
    if(!CRingBufferPriv_timedLock(&rb->base->mutex, timeoutMs == RB_WAIT_INFINITE ? RB_WAIT_INFINITE : timeoutMs - Rb_Stopwatch_elapsedMs(&sw))){
        Rb_Stopwatch_free(&sw);
        return RB_TIMEOUT;
    }

    // Checkpoint
    if(!rb->base->enabled) {
        LOCK_RELEASE
        ;
        WRITE_RELEASE
        ;
        Rb_Stopwatch_free(&sw);
        return 0;
    }

    if(mode == eRB_WRITE_BLOCK_FULL) {
        uint32_t bytesRemaining = size;
        uint32_t bytesFree = 0;

        while(bytesRemaining) {
            // Wait until some space is free
            while((bytesFree = Rb_RingBuffer_getBytesFree(rb->buffer)) == 0
                    && rb->base->enabled) {
                if(!CRingBufferPriv_timedWait(&rb->base->readCV, &rb->base->mutex, timeoutMs == RB_WAIT_INFINITE ? RB_WAIT_INFINITE :  timeoutMs - Rb_Stopwatch_elapsedMs(&sw))){
                   LOCK_RELEASE
                   ;
                   WRITE_RELEASE
                   ;
                   Rb_Stopwatch_free(&sw);
                   return RB_TIMEOUT;
               }
            }

            // Checkpoint
            if(!rb->base->enabled) {
                LOCK_RELEASE
                ;
                WRITE_RELEASE
                ;
                Rb_Stopwatch_free(&sw);
                return size - bytesRemaining;
            }

            const uint32_t toWrite =
                    bytesRemaining < bytesFree ? bytesRemaining : bytesFree;

            Rb_RingBuffer_write(rb->buffer, data + (size - bytesRemaining),
                    toWrite);

            bytesRemaining -= toWrite;

            pthread_cond_broadcast(&rb->base->writeCV);
        }

        bytesWritten = size - bytesRemaining;
    } else {
        if(mode == eRB_WRITE_WRITE_SOME) {
            // Write as much data as we can without blocking
            uint32_t free = Rb_RingBuffer_getBytesFree(rb->buffer);

            size = size > free ? free : size;
        }

        if(size) {
            bytesWritten = Rb_RingBuffer_write(rb->buffer, data, size);

            pthread_cond_broadcast(&rb->base->writeCV);
        }
    }

    LOCK_RELEASE
    ;
    WRITE_RELEASE
    ;
    Rb_Stopwatch_free(&sw);
    return bytesWritten;
}

int32_t Rb_CRingBuffer_write(Rb_CRingBufferHandle handle, const uint8_t* data,
        uint32_t size, Rb_CRingBuffer_WriteMode mode) {
    return Rb_CRingBuffer_writeTimed(handle, data, size, mode, RB_WAIT_INFINITE);
}

int32_t Rb_CRingBuffer_getBytesUsed(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    const int32_t res = Rb_RingBuffer_getBytesUsed(rb->buffer);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_CRingBuffer_getBytesFree(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    const int32_t res = Rb_RingBuffer_getBytesFree(rb->buffer);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_CRingBuffer_getCapacity(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    const int32_t res = Rb_RingBuffer_getCapacity(rb->buffer);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_CRingBuffer_disable(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    rb->base->enabled = 0;

    pthread_cond_broadcast(&rb->base->readCV);
    pthread_cond_broadcast(&rb->base->writeCV);

    LOCK_RELEASE
    ;

    return 0;
}

int32_t Rb_CRingBuffer_enable(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    rb->base->enabled = 1;

    LOCK_RELEASE
    ;

    return 0;
}

int32_t Rb_CRingBuffer_isEnabled(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    const int32_t res = rb->base->enabled;

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_CRingBuffer_clear(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    Rb_RingBuffer_clear(rb->buffer);

    pthread_cond_broadcast(&rb->base->readCV);

    LOCK_RELEASE
    ;

    return 0;
}

int32_t Rb_CRingBuffer_isFull(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    int32_t res = Rb_RingBuffer_isFull(rb->buffer);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_CRingBuffer_isEmpty(Rb_CRingBufferHandle handle) {
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    int32_t res = Rb_RingBuffer_isEmpty(rb->buffer);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_CRingBuffer_resize(Rb_CRingBufferHandle handle, uint32_t capacity){
    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    int32_t res = Rb_RingBuffer_resize(rb->buffer, capacity);

    LOCK_RELEASE
    ;

    return res;
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

    CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
    if(rb == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE
    ;

    int32_t bytesUsed = Rb_RingBuffer_getBytesUsed(rb->buffer);
    int32_t capacity = Rb_RingBuffer_getCapacity(rb->buffer);

    float res = 100 * (float) bytesUsed / (float) capacity;

    LOCK_RELEASE
    ;

    return res;
}
