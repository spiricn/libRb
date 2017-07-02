/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Common.h"
#include "rb/Utils.h"
#include "rb/List.h"
#include "rb/ConsumerProducer.h"
#include "rb/priv/ErrorPriv.h"

#include <pthread.h>
#include <stdbool.h>

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
} ConsProdContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static ConsProdContext* ConsProdPriv_getContext(
        Rb_ConsumerProducerHandle handle);

static int32_t ConsProdPriv_notifyWritten(ConsProdContext* cp);

static int32_t ConsProdPriv_notifyRead(ConsProdContext* cp);

static bool ConsumerProducerPriv_timedWait(pthread_cond_t* cv,
        pthread_mutex_t* mutex, int64_t ms);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_ConsumerProducerHandle Rb_ConsumerProducer_new() {
    int32_t rc;

    ConsProdContext* cp = RB_CALLOC(sizeof(ConsProdContext));

    cp->magic = CONSUMER_PRODUCER_MAGIC;

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
        Rb_ConsumerProducerConditionFnc fnc, void* arg) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = pthread_mutex_lock(&cp->readMutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    rc = pthread_mutex_lock(&cp->mutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    while (!fnc(arg)) {
        pthread_cond_wait(&cp->readCV, &cp->mutex);
    }

    return RB_OK;
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

int32_t Rb_ConsumerProducer_acquireWriteLock(Rb_ConsumerProducerHandle handle,
        Rb_ConsumerProducerConditionFnc fnc, void* arg) {
    int32_t rc;

    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    rc = pthread_mutex_lock(&cp->writeMutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    rc = pthread_mutex_lock(&cp->mutex);
    if (rc != 0) {
        RB_ERRC(RB_ERROR, "pthread_mutex_lock failed");
    }

    while (!fnc(arg)) {
        pthread_cond_wait(&cp->writeCV, &cp->mutex);
    }

    return RB_OK;
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

bool ConsumerProducerPriv_timedWait(pthread_cond_t* cv, pthread_mutex_t* mutex,
        int64_t ms) {
    if (ms == RB_WAIT_INFINITE) {
        pthread_cond_wait(cv, mutex);

        return true;
    } else {
        if (ms <= 0) {
            return false;
        }

        struct timespec time;
        Rb_Utils_getOffsetTime(&time, ms);

        return pthread_cond_timedwait(cv, mutex, &time) == 0;
    }
}
