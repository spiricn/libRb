#ifndef RB_RING_BUFFER_H_
#define RB_RING_BUFFER_H_

/********************************************************/
/*                 Includes                             */
/********************************************************/

#include "rb/Common.h"
#include <stdint.h>

/********************************************************/
/*                 Typedefs                             */
/********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Rb_RingBufferHandle;

/********************************************************/
/*                 Functions Declarations               */
/********************************************************/

/**
 * Creates new ring buffer object.
 *
 * @param[in] capacity Buffer capacity.
 * @return Buffer object on sucess, NULL on failure.
 */
Rb_RingBufferHandle Rb_RingBuffer_new(uint32_t capacity);

/**
 * Creates ring buffer from an already allocated memory block (may be shared between processes).
 *
 * @param[in] data Memory block where the buffer was allocated.
 * @param[in] size Size of the provided memory block.
 * @param[in] init If set to 1 initializes the memory (creates a new buffer object). If set to 0 assumes the buffer was already created.
 * @return Buffer object on sucess, NULL on failure
 */
Rb_RingBufferHandle Rb_RingBuffer_fromSharedMemory(void* data, uint32_t size,
        int init);
/**
 * Deallocate a ring buffer, and, as a side effect, set the pointer to NULL.
 *
 * @param[in,out] handle Pointer to a valid ring buffer handle.
 * @return RB_ERROR on failure, RB_OK on success.
 */
int32_t Rb_RingBuffer_free(Rb_RingBufferHandle* handle);

/**
 * Reset a ring buffer to its initial state (empty).
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_OK on success.
 */
int32_t Rb_RingBuffer_clear(Rb_RingBufferHandle handle);

/**
 * The usable capacity of the ring buffer, in bytes.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, ring buffer capacity otherwise
 */
int32_t Rb_RingBuffer_getCapacity(Rb_RingBufferHandle handle);

/**
 * The number of free/available bytes in the ring buffer. This value
 * is never larger than the ring buffer's usable capacity.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, number of free bytes otherwise.
 */
int32_t Rb_RingBuffer_getBytesFree(Rb_RingBufferHandle handle);

/**
 * The number of bytes currently being used in the ring buffer. This
 * value is never larger than the ring buffer's usable capacity.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, number of buffer bytes used otherwise.
 */
int32_t Rb_RingBuffer_getBytesUsed(Rb_RingBufferHandle handle);

/**
 * Checks if the ring buffer is full.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, 1 if the buffer is full 0 otherwise.
 */
int32_t Rb_RingBuffer_isFull(Rb_RingBufferHandle handle);

/**
 * Checks if the ring buffer is empty.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, 1 if the buffer is empty 0 otherwise.
 */
int32_t Rb_RingBuffer_isEmpty(Rb_RingBufferHandle handle);

/**
 * Copy bytes from a contiguous memory area into the ring buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] src Source buffer.
 * @param[in] count Source buffer size.
 * @return Negative value on failure, number of bytes written otherwise.
 */
int32_t Rb_RingBuffer_write(Rb_RingBufferHandle handle, const void* src,
        uint32_t count);

/**
 * Reads data from the buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[out] dst Destination buffer.
 * @param[in] count Destination buffer size.
 * @return Negative value on failure, number of bytes read otherwise.
 */
int32_t Rb_RingBuffer_read(Rb_RingBufferHandle handle, void* dst, uint32_t count);

/**
 * Grows or shrinks ring buffers internal buffer
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] capacity New ring buffer size.
 * @return Negative value on failure, RB_OK otherwise.
 */
int32_t Rb_RingBuffer_resize(Rb_RingBufferHandle handle, uint32_t capacity);

#ifdef __cplusplus
}
#endif

#endif
