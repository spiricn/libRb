/**
 * @file ConcurrentRingBuffer.h
 * @author Nikola Spiric <nikola.spiric@rt-rk.com>
 */

#ifndef RINGBUF_H_
#define RINGBUF_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

struct ringbuf_t;

typedef struct ringbuf_t* RingBuffer;

/**
 * Creates new ring buffer object.
 *
 * @param[in] capacity Buffer capacity.
 * @return Buffer object on sucess, NULL on failure.
 */
RingBuffer RingBuffer_new(uint32_t capacity);

/**
 * Creates ring buffer from an already allocated memory block (may be shared between processes).
 *
 * @param[in] data Memory block where the buffer was allocated.
 * @param[in] size Size of the provided memory block.
 * @param[in] init If set to 1 initializes the memory (creates a new buffer object). If set to 0 assumes the buffer was already created.
 * @return Buffer object on sucess, NULL on failure
 */
RingBuffer RingBuffer_fromSharedMemory(void* data, uint32_t size, int init);

/**
 * The size of the internal buffer, in bytes. One or more bytes may be
 * unusable in order to distinguish the "buffer full" state from the
 * "buffer empty" state.
 * For the usable capacity of the ring buffer, use the
 * RingBuffer_getCapacity function.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, buffer size otherwise.
 */
int32_t RingBuffer_getSize(const RingBuffer rb);

/**
 * Deallocate a ring buffer, and, as a side effect, set the pointer to NULL.
 *
 * @param[in,out] rb Pointer to a valid ring buffer handle.
 * @return -1 on failure, 0 on success.
 */
int32_t RingBuffer_free(RingBuffer* rb);

/**
 * Reset a ring buffer to its initial state (empty).
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, 0 on success.
 */
int32_t RingBuffer_clear(RingBuffer rb);

/**
 * The usable capacity of the ring buffer, in bytes. Note that this
 * value may be less than the ring buffer's internal buffer size, as
 * returned by RingBuffer_getSize.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, ring buffer capacity otherwise
 */
int32_t RingBuffer_getCapacity(const RingBuffer rb);

/**
 * The number of free/available bytes in the ring buffer. This value
 * is never larger than the ring buffer's usable capacity.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return
 */
int32_t RingBuffer_getBytesFree(const RingBuffer rb);

/**
 * The number of bytes currently being used in the ring buffer. This
 * value is never larger than the ring buffer's usable capacity.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, number of buffer bytes used otherwise.
 */
int32_t RingBuffer_getBytesUsed(const RingBuffer rb);

/**
 * Checks if the ring buffer is full.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, 1 if the buffer is full 0 otherwise.
 */
int32_t RingBuffer_isFull(const RingBuffer);

/**
 * Checks if the ring buffer is empty.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, 1 if the buffer is empty 0 otherwise.
 */
int32_t RingBuffer_isEmpty(const RingBuffer);

/**
 * Copy bytes from a contiguous memory area into the ring buffer.
 *
 * @param[in] rb Valid ring buffer handle.
 * @param[in] src Source buffer.
 * @param[in] count Source buffer size.
 * @return -1 on failure, number of bytes written otherwise.
 */
int32_t RingBuffer_write(RingBuffer rb, const void* src, uint32_t count);

/**
 * Reads data from the buffer.
 *
 * @param[in] rb Valid ring buffer handle.
 * @param[out] dst Destination buffer.
 * @param[in] count Destination buffer size.
 * @return -1 on failure, number of bytes read otherwise.
 */
int32_t RingBuffer_read(RingBuffer rb, void* dst, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /* RINGBUF_H_ */
