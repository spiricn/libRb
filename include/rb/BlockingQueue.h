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

Rb_BlockingQueueHandle Rb_BlockingQueue_new(int32_t messageSize,
        int32_t capacity);

int32_t Rb_BlockingQueue_free(Rb_BlockingQueueHandle* handle);

int32_t Rb_BlockingQueue_put(Rb_BlockingQueueHandle handle,
        const void* message);

int32_t Rb_BlockingQueue_get(Rb_BlockingQueueHandle handle, void* message);

int32_t Rb_BlockingQueue_peek(Rb_BlockingQueueHandle handle, void* message);

int32_t Rb_BlockingQueue_resize(Rb_BlockingQueueHandle handle, int32_t capacity);

int32_t Rb_BlockingQueue_clear(Rb_BlockingQueueHandle handle);

int32_t Rb_BlockingQueue_isFull(Rb_BlockingQueueHandle handle);

int32_t Rb_BlockingQueue_isEmpty(Rb_BlockingQueueHandle handle);

int32_t Rb_BlockingQueue_getNumMessages(Rb_BlockingQueueHandle handle);

int32_t Rb_BlockingQueue_getCapacity(Rb_BlockingQueueHandle handle);

#ifdef __cplusplus
}
#endif

#endif
