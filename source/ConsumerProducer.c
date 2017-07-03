/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Common.h"
#include "rb/Utils.h"
#include "rb/List.h"
#include "rb/ConsumerProducer.h"
#include "rb/priv/ErrorPriv.h"
#include "rb/Stopwatch.h"

#include <pthread.h>
#include <stdbool.h>
#include <errno.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define CONSUMER_PRODUCER_MAGIC ( 0xAB541111 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    uint32_t magic;
    pthread_mutex_t mutex;
    pthread_mutex_t readMutex;
    pthread_mutex_t writeMutex;
    pthread_cond_t readCV;
    pthread_cond_t writeCV;
    bool enabled;
} ConsProdContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static ConsProdContext* ConsProdPriv_getContext(
        Rb_ConsumerProducerHandle handle);

static int32_t ConsProdPriv_notifyWritten(ConsProdContext* cp);

static int32_t ConsProdPriv_notifyRead(ConsProdContext* cp);

static int32_t ConsumerProducerPriv_timedWait(pthread_cond_t* cv,
        pthread_mutex_t* mutex, int64_t ms);

static int32_t ConsumerProducerPriv_timedLock(pthread_mutex_t* mutex, int64_t ms);

static int32_t ConsProdPriv_acquireLock(ConsProdContext* cp,
        pthread_mutex_t* mutex, pthread_cond_t* cv,
        Rb_ConsumerProducerConditionFnc fnc, void* arg, int64_t timeoutMs);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_ConsumerProducerHandle Rb_ConsumerProducer_new() {
    int32_t rc;

    ConsProdContext* cp = RB_CALLOC(sizeof(ConsProdContext));

    cp->magic = CONSUMER_PRODUCER_MAGIC;
    cp->enabled = true;

    rc = pthread_mutex_init(&cp->mutex, NULL);
    if (rc != 0) {
        RB_ERR("pthread_mutex_init failed");
        return NULL;
    }

    rc = pthread_mutex_init(&cp->readMutex, NULL);
    if (rc != 0) {
        RB_ERR("pthread_mutex_init failed");
        return NULL;
    }

    rc = pthread_mutex_init(&cp->writeMutex, NULL);
    if (rc != 0) {
        RB_ERR("pthread_mutex_init failed");
        return NULL;
    }

    rc = pthread_cond_init(&cp->readCV, NULL);
    if (rc != 0) {
        RB_ERR("pthread_cond_init failed");
        return NULL;
    }

    rc = pthread_cond_init(&cp->writeCV, NULL);
    if (rc != 0) {
        RB_ERR("pthread_cond_init failed");
        return NULL;
    }

    return cp;
}

int32_t Rb_ConsumerProducer_free(Rb_ConsumerProducerHandle* handle) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle ? *handle : NULL);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = pthread_mutex_destroy(&cp->mutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_destroy failed");
    }

    rc = pthread_mutex_destroy(&cp->writeMutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_destroy failed");
    }

    rc = pthread_mutex_destroy(&cp->readMutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_destroy failed");
    }

    rc = pthread_cond_destroy(&cp->readCV);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_cond_destroy failed");
    }

    rc = pthread_cond_destroy(&cp->writeCV);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_cond_destroy failed");
    }

    RB_FREE(&cp);
    *handle = NULL;

    return RB_OK;
}

int32_t Rb_ConsumerProducer_acquireLock(Rb_ConsumerProducerHandle handle) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = pthread_mutex_lock(&cp->mutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    return RB_OK;
}

int32_t Rb_ConsumerProducer_releaseLock(Rb_ConsumerProducerHandle handle) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = pthread_mutex_unlock(&cp->mutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    return RB_OK;
}

int32_t Rb_ConsumerProducer_acquireReadLock(Rb_ConsumerProducerHandle handle,
        Rb_ConsumerProducerConditionFnc fnc, void* arg, int64_t timeoutMs) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if(cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return ConsProdPriv_acquireLock(cp, &cp->readMutex, &cp->readCV, fnc, arg, timeoutMs);
}

int32_t Rb_ConsumerProducer_releaseReadLock(Rb_ConsumerProducerHandle handle,
        bool notify) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    if (notify) {
        rc = ConsProdPriv_notifyRead(cp);
        if (rc != RB_OK) {
            return rc;
        }
    }

    rc = pthread_mutex_unlock(&cp->mutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    rc = pthread_mutex_unlock(&cp->readMutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    return RB_OK;
}

int32_t ConsProdPriv_acquireLock(ConsProdContext* cp, pthread_mutex_t* mutex,
        pthread_cond_t* cv, Rb_ConsumerProducerConditionFnc fnc, void* arg, int64_t timeoutMs) {
    int32_t rc;

    // Start measuring time
    Rb_StopwatchHandle sw = Rb_Stopwatch_new();
    Rb_Stopwatch_start(sw);

    // Attempt to lock reader/writer mutex in the given time period
    rc = ConsumerProducerPriv_timedLock(mutex, timeoutMs - Rb_Stopwatch_elapsedMs(sw));
    if (rc == RB_TIMEOUT) {
        // Did not manage to lock it in time
        Rb_Stopwatch_free(&sw);

        return rc;
    }
    else if(rc != RB_OK){
        Rb_Stopwatch_free(&sw);

        RB_ERRC(rc, "ConsumerProducerPriv_timedLock failed");
    }

    // Check if we got disabled in the meantime
    if(!cp->enabled) {
        Rb_Stopwatch_free(&sw);

        rc = pthread_mutex_unlock(mutex);
        if(rc != 0) {
            RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
        }

        return RB_DISABLED;
    }

    // Attempt to acquire global lock in the given time period
    rc = ConsumerProducerPriv_timedLock(&cp->mutex, timeoutMs - Rb_Stopwatch_elapsedMs(sw));
    if(rc == RB_TIMEOUT){
        Rb_Stopwatch_free(&sw);

        // Did not manage to lock it in time (so unlock the reader/writer mutex and return
        rc = pthread_mutex_unlock(mutex);
        if (rc != 0) {
            RB_ERRC(rc, "pthread_mutex_unlock failed");
        }
        return RB_TIMEOUT;
    }
    else if (rc != RB_OK) {
        Rb_Stopwatch_free(&sw);

        RB_ERRC(rc, "ConsumerProducerPriv_timedLock failed");
    }

    // Check if we got disabled in the meantime
    if(!cp->enabled) {
        Rb_Stopwatch_free(&sw);

        rc = pthread_mutex_unlock(mutex);
        if(rc != 0) {
            RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
        }

        rc = pthread_mutex_unlock(&cp->mutex);
        if(rc != 0) {
            RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
        }

        return RB_DISABLED;
    }

    // Wait until:
    // - We're either ready for reading/writing
    // - We timeout
    // - We get disable
    while(true){
        bool rwReady = fnc(arg);
        if (rwReady) {
            break;
        }

        // Not ready yet so wait
        rc = ConsumerProducerPriv_timedWait(cv, &cp->mutex, timeoutMs - Rb_Stopwatch_elapsedMs(sw));
        if(rc == RB_TIMEOUT){
            Rb_Stopwatch_free(&sw);

            rc = pthread_mutex_unlock(mutex);
            if (rc != 0) {
                RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
            }

            rc = pthread_mutex_unlock(&cp->mutex);
            if (rc != 0) {
                RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
            }

            return RB_TIMEOUT;
        }
        else if(rc != RB_OK){
            Rb_Stopwatch_free(&sw);

            RB_ERRC(rc, "ConsumerProducerPriv_timedWait failed:\n%s", Rb_getLastErrorMessage());
        }
    }

    // Check if we got disabled in the meantime
    if(!cp->enabled) {
        Rb_Stopwatch_free(&sw);

        rc = pthread_mutex_unlock(mutex);
        if(rc != 0) {
            RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
        }

        rc = pthread_mutex_unlock(&cp->mutex);
        if(rc != 0) {
            RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
        }

        return RB_DISABLED;
    }

    return RB_OK;
}

int32_t Rb_ConsumerProducer_acquireWriteLock(Rb_ConsumerProducerHandle handle,
        Rb_ConsumerProducerConditionFnc fnc, void* arg, int64_t timeoutMs) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if(cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return ConsProdPriv_acquireLock(cp, &cp->writeMutex, &cp->writeCV, fnc, arg, timeoutMs);
}

int32_t Rb_ConsumerProducer_releaseWriteLock(Rb_ConsumerProducerHandle handle,
        bool notify) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    if (notify) {
        rc = ConsProdPriv_notifyWritten(cp);
        if (rc != RB_OK) {
            return rc;
        }
    }

    rc = pthread_mutex_unlock(&cp->mutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    rc = pthread_mutex_unlock(&cp->writeMutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    return RB_OK;
}

int32_t Rb_ConsumerProducer_notifyRead(Rb_ConsumerProducerHandle handle) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return ConsProdPriv_notifyRead(cp);
}

int32_t Rb_ConsumerProducer_notifyWritten(Rb_ConsumerProducerHandle handle) {
    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return ConsProdPriv_notifyWritten(cp);
}

int32_t ConsProdPriv_notifyWritten(ConsProdContext* cp) {
    int32_t rc;

    rc = pthread_cond_broadcast(&cp->readCV);
    if (rc != 0) {
        RB_ERR("pthread_cond_broadcast failed: %d", rc);
        return RB_ERROR;
    }

    return RB_OK;
}

int32_t ConsProdPriv_notifyRead(ConsProdContext* cp) {
    int32_t rc;

    rc = pthread_cond_broadcast(&cp->writeCV);
    if (rc != 0) {
        RB_ERR("pthread_cond_broadcast failed: %d", rc);
        return RB_ERROR;
    }

    return RB_OK;
}

int32_t Rb_ConsumerProducer_disable(Rb_ConsumerProducerHandle handle) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if(cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(handle);
    if(rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    if(cp->enabled) {
        cp->enabled = false;

        rc = Rb_ConsumerProducer_notifyRead(handle);
        if(rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_notifyRead failed");
        }

        rc = Rb_ConsumerProducer_notifyWritten(handle);
        if(rc != RB_OK) {
            RB_ERRC(rc, "Rb_ConsumerProducer_notifyWritten failed");
        }
    }

    rc = Rb_ConsumerProducer_releaseLock(handle);
    if(rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return RB_OK;
}

int32_t Rb_ConsumerProducer_enable(Rb_ConsumerProducerHandle handle) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if(cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = Rb_ConsumerProducer_acquireLock(handle);
    if(rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_acquireLock failed");
    }

    if(!cp->enabled) {
        cp->enabled = true;
    }

    rc = Rb_ConsumerProducer_releaseLock(handle);
    if(rc != RB_OK) {
        RB_ERRC(rc, "Rb_ConsumerProducer_releaseLock failed");
    }

    return RB_OK;
}

ConsProdContext* ConsProdPriv_getContext(Rb_ConsumerProducerHandle handle) {
    if (handle == NULL) {
        return NULL;
    }

    ConsProdContext* cp = (ConsProdContext*) handle;
    if (cp->magic != CONSUMER_PRODUCER_MAGIC) {
        return NULL;
    }

    return cp;
}

int32_t ConsumerProducerPriv_timedWait(pthread_cond_t* cv, pthread_mutex_t* mutex,
        int64_t ms) {
    int32_t rc;

    if (ms == RB_WAIT_INFINITE) {
        rc = pthread_cond_wait(cv, mutex);
        if(rc != 0){
            RB_ERRC(RB_ERROR, "pthread_cond_wait failed");
        }

        return RB_OK;
    } else {
        if (ms <= 0) {
            return RB_TIMEOUT;
        }

        struct timespec time;
        Rb_Utils_getOffsetTime(&time, ms);

        rc = pthread_cond_timedwait(cv, mutex, &time) == 0;
        if (rc == 0) {
            return RB_OK;
        } else if (rc == ETIMEDOUT) {
            return RB_TIMEOUT;
        } else {
            RB_ERRC(RB_ERROR, "pthread_cond_timedwait failed");
        }
    }
}


static int32_t ConsumerProducerPriv_timedLock(pthread_mutex_t* mutex, int64_t ms){
    int32_t rc;

    if (ms == RB_WAIT_INFINITE) {
        rc = pthread_mutex_lock(mutex);
        if(rc != 0){
            return RB_ERROR;
        }

        return RB_OK;
    } else {
        if (ms <= 0) {
            return RB_TIMEOUT;
        }

        struct timespec time;
        Rb_Utils_getOffsetTime(&time, ms);

        rc = pthread_mutex_timedlock(mutex, &time);
        if (rc == 0) {
            return RB_OK;
        } else if (rc == ETIMEDOUT) {
            return RB_TIMEOUT;
        } else {
            return RB_ERROR;
        }
    }
}
