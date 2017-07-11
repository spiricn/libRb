#ifndef RB_BLOCKINGQUEUE_H_
#define RB_BLOCKINGQUEUE_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <stdint.h>

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Rb_BlockingQueueHandle;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/**
 * TODO
 */
Rb_BlockingQueueHandle Rb_BlockingQueue_new(int32_t messageSize,
        int32_t capacity);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_free(Rb_BlockingQueueHandle* handle);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_put(Rb_BlockingQueueHandle handle,
        const void* message);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_putTimed(Rb_BlockingQueueHandle handle,
        const void* message, int64_t timeoutMs);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_get(Rb_BlockingQueueHandle handle, void* message);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_getTimed(Rb_BlockingQueueHandle handle, void* message, int64_t timeoutMs);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_peek(Rb_BlockingQueueHandle handle, void* message);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_peekTimed(Rb_BlockingQueueHandle handle, void* message, int64_t timeoutMs);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_resize(Rb_BlockingQueueHandle handle, int32_t capacity);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_clear(Rb_BlockingQueueHandle handle);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_isFull(Rb_BlockingQueueHandle handle);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_isEmpty(Rb_BlockingQueueHandle handle);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_getNumMessages(Rb_BlockingQueueHandle handle);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_getCapacity(Rb_BlockingQueueHandle handle);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_disable(Rb_BlockingQueueHandle handle);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_enable(Rb_BlockingQueueHandle handle);

/**
 * TODO
 */
int32_t Rb_BlockingQueue_isEnabled(Rb_BlockingQueueHandle handle);

#ifdef __cplusplus
}
#endif

#endif
