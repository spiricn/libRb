#ifndef RB_BUFFERQUEUE_H_
#define RB_BUFFERQUEUE_H_

/********************************************************/
/*                 Includes                             */
/********************************************************/

#include <rb/Common.h>

#include <stdint.h>
#include <stdbool.h>

/********************************************************/
/*                 Defines                              */
/********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#define Rb_BUFFER_QUEUE_INVALID_INDEX ( -1 )

/********************************************************/
/*                 Typedefs                             */
/********************************************************/

typedef void* Rb_BufferQueueHandle;

/********************************************************/
/*                 Functions Declarations               */
/********************************************************/

Rb_BufferQueueHandle Rb_BufferQueue_new(int32_t numBuffers, int32_t bufferSize);

int32_t Rb_BufferQueue_free(Rb_BufferQueueHandle* handle);

int32_t Rb_BufferQueue_addBuffer(Rb_BufferQueueHandle handle, void* buffer,
        int32_t size, int32_t index);

int32_t Rb_BufferQueue_containsBuffer(Rb_BufferQueueHandle handle, int32_t index);

int32_t Rb_BufferQueue_getBuffer(Rb_BufferQueueHandle handle, int32_t index, void** data, int32_t* size);

int32_t Rb_BufferQueue_dequeueInputBuffer(Rb_BufferQueueHandle handle,
        int32_t* index, int32_t timeoutMs);

int32_t Rb_BufferQueue_queueInputBuffer(Rb_BufferQueueHandle handle,
        int32_t index);

int32_t Rb_BufferQueue_dequeueFreeBuffer(Rb_BufferQueueHandle handle,
        int32_t* index, int32_t timeoutMs);

int32_t Rb_BufferQueue_queueFreeBuffer(Rb_BufferQueueHandle handle,
        int32_t index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
