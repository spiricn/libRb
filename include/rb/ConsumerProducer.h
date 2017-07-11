#ifndef CONSUMER_PRODUCER_H_
#define CONSUMER_PRODUCER_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Rb_ConsumerProducerHandle;

typedef bool (*Rb_ConsumerProducerConditionFnc)(void* arg);

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/*
 * TODO
 */
Rb_ConsumerProducerHandle Rb_ConsumerProducer_new();

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_free(Rb_ConsumerProducerHandle* handle);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_acquireReadLock(Rb_ConsumerProducerHandle handle,
        Rb_ConsumerProducerConditionFnc fnc, void* arg, int64_t timeoutMs);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_releaseReadLock(Rb_ConsumerProducerHandle handle,
        bool notify);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_acquireWriteLock(Rb_ConsumerProducerHandle handle,
        Rb_ConsumerProducerConditionFnc fnc, void* arg, int64_t timeoutMs);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_releaseWriteLock(Rb_ConsumerProducerHandle handle,
        bool notify);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_acquireLock(Rb_ConsumerProducerHandle handle);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_releaseLock(Rb_ConsumerProducerHandle handle);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_notifyRead(Rb_ConsumerProducerHandle handle);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_notifyWritten(Rb_ConsumerProducerHandle handle);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_disable(Rb_ConsumerProducerHandle handle);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_enable(Rb_ConsumerProducerHandle handle);

/*
 * TODO
 */
int32_t Rb_ConsumerProducer_isEnabled(Rb_ConsumerProducerHandle handle);

#ifdef __cplusplus
}
#endif

#endif
