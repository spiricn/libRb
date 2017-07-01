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

#define LOCK_ACQUIRE do{ pthread_mutex_lock(&cp->mutex); }while(0)
#define LOCK_RELEASE do{  pthread_mutex_unlock(&cp->mutex); }while(0)
#define READ_LOCK do{  pthread_mutex_lock(&cp->readMutex);}while(0)
#define READ_RELEASE do{  pthread_mutex_unlock(&cp->readMutex); }while(0)
#define WRITE_LOCK do{  pthread_mutex_lock(&cp->writeMutex); }while(0)
#define WRITE_RELEASE do{ pthread_mutex_unlock(&cp->writeMutex); }while(0)

#define CONSUMER_PRODUCER_MAGIC ( 0xAB541111 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    int32_t magic;
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

static bool ConsumerProducerPriv_timedWait(pthread_cond_t* cv,
        pthread_mutex_t* mutex, int64_t ms);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_ConsumerProducerHandle Rb_ConsumerProducer_new() {
    ConsProdContext* cp = RB_CALLOC(sizeof(ConsProdContext));

    cp->magic = CONSUMER_PRODUCER_MAGIC;

    pthread_mutex_init(&cp->mutex, NULL);
    pthread_mutex_init(&cp->readMutex, NULL);
    pthread_mutex_init(&cp->writeMutex, NULL);
    pthread_cond_init(&cp->readCV, NULL);
    pthread_cond_init(&cp->writeCV, NULL);

    return cp;
}

int32_t Rb_ConsumerProducer_free(Rb_ConsumerProducerHandle* handle) {
    ConsProdContext* cp = ConsProdPriv_getContext(handle ? *handle : NULL);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    pthread_mutex_destroy(&cp->mutex);
    pthread_mutex_destroy(&cp->writeMutex);
    pthread_mutex_destroy(&cp->readMutex);
    pthread_cond_destroy(&cp->readCV);
    pthread_cond_destroy(&cp->writeCV);

    RB_FREE(&cp);

    *handle = NULL;

    return RB_OK;
}

int32_t Rb_ConsumerProducer_acquireLock(Rb_ConsumerProducerHandle handle) {
    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    LOCK_ACQUIRE
    ;

    return RB_OK;
}

int32_t Rb_ConsumerProducer_releaseLock(Rb_ConsumerProducerHandle handle) {
    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    LOCK_RELEASE
    ;

    return RB_OK;
}

int32_t Rb_ConsumerProducer_acquireReadLock(Rb_ConsumerProducerHandle handle,
        Rb_ConsumerProducerConditionFnc fnc, void* arg) {
    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    READ_LOCK
    ;

    LOCK_ACQUIRE
    ;

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
        rc = pthread_cond_broadcast(&cp->writeCV);
        if (rc != 0) {
            RB_ERR("pthread_cond_broadcast failed: %d", rc);
            return RB_ERROR;
        }
    }

    LOCK_RELEASE
    ;

    READ_RELEASE
    ;

    return RB_OK;
}

int32_t Rb_ConsumerProducer_acquireWriteLock(Rb_ConsumerProducerHandle handle,
        Rb_ConsumerProducerConditionFnc fnc, void* arg) {
    ConsProdContext* cp = ConsProdPriv_getContext(handle);
    if (cp == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    WRITE_LOCK
    ;

    LOCK_ACQUIRE
    ;

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
        rc = pthread_cond_broadcast(&cp->readCV);
        if (rc != 0) {
            RB_ERR("pthread_cond_broadcast failed: %d", rc);
            return RB_ERROR;
        }
    }

    LOCK_RELEASE
    ;

    WRITE_RELEASE
    ;

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
