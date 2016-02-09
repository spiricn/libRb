/**
 * @file ConcurrentRingBuffer.h
 * @author Nikola Spiric <nikola.spiric@rt-rk.com>
 */

#ifndef CONCURRENTRINGBUFFER_H_
#define CONCURRENTRINGBUFFER_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <pthread.h>

typedef enum{
	/**
	 * Block until there is enough data to fill the entire buffer
	 */
	eREAD_BLOCK_FULL,

	/**
	 * Blocks until there is at least some data available
	 */
	eREAD_BLOCK_PARTIAL,

	/**
	 * Return immidietly without blocking (may return zero)
	 */
	eREAD_BLOCK_NONE
} CRingBuffer_ReadMode; // </ReadMode>

typedef enum{
	/**
	 * Blocks until there is enough free space to write the total ammount of data provided
	 */
	eWRITE_BLOCK_FULL,

	/**
	 * Does not block, writes all data possibly overwriting old data
	 */
	eWRITE_OVERFLOW,

	/**
	 * Does not block, attempts to write all data if there is enough space, otherwise writes at least some data
	 */
	eWRITE_WRITE_SOME
} CRingBuffer_WriteMode;

struct ConcurrentRingBuffer_t;

typedef struct ConcurrentRingBuffer_t* CRingBuffer;

/**
 * Creates new concurrent ring buffer object
 *
 * @param[in] size Buffer capacity
 * @return Buffer object on sucess, NULL on failure
 */
CRingBuffer CRingBuffer_new(uint32_t size);

/**
 * Creates ring buffer from an already allocated memory block (may be shared between processes).
 *
 * @param[in] memory Memory block where the buffer was allocated.
 * @param[in] size Size of the provided memory block.
 * @param[in] init If set to 1 initializes the memory (creates a new buffer object). If set to 0 assumes the buffer was already created.
 * @return Buffer object on sucess, NULL on failure
 */
CRingBuffer CRingBuffer_fromSharedMemory(void* memory, uint32_t size, int init);

/**
 * Frees a buffer object created via 'CRingBuffer_new' or 'CRingBuffer_fromSharedMemory' functions.
 *
 * @param[in,out] rb Valid ring buffer handle.
 * @return -1 on failure, 0 on success
 */
int32_t CRingBuffer_free(CRingBuffer* rb);

/**
 * Reads data from the buffer. May block depending on the read mode.
 *
 * @param[in] rb Valid ring buffer handle.
 * @param[in] data Destination buffer.
 * @param[in] size Size of the destination buffer.
 * @param[in] mode Mode which decides the behavior of the function call. See 'CRingBuffer_ReadMode' enumeration for more info.
 * @return -1 on failure, number of bytes read otherwise.
 */
int32_t CRingBuffer_read(CRingBuffer rb, uint8_t* data, uint32_t size, CRingBuffer_ReadMode mode);

/**
 * Writes data to the buffer. May block depending on the write mode.
 *
 * @param[in] rb Valid ring buffer handle.
 * @param[in] data Source buffer.
 * @param[in] size Size of the source buffer.
 * @param[in] mode Mode which decides the behavior of the function call. See 'CRingBuffer_WriteMode' enumeration for more info.
 * @return -1 on failure, number of bytes written otherwise.
 */
int32_t CRingBuffer_write(CRingBuffer rb,  const uint8_t* data, uint32_t size, CRingBuffer_WriteMode mode);

/**
 * Gets the number of bytes currently contained in the buffer.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, number of bytes contained otherwise.
 */
int32_t CRingBuffer_getBytesUsed(CRingBuffer rb);

/**
 * Gets the number of free space left in the buffer.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, number of bytes free otherwise.
 */
int32_t CRingBuffer_getBytesFree(CRingBuffer rb);

/**
 * Gets the total capacity of the buffer.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, ring buffer capacity otherwise.
 */
int32_t CRingBuffer_getSize(const CRingBuffer rb);

/**
 * Disables the ring buffer. All currently blocking operations (e.g. 'CRingBuffer_read' or 'CRingBuffer_write' calls) will return immediately.
 * After this call any further 'CRingBuffer_read' or 'CRingBuffer_write' calls will return 0 without blocking.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, 0 otherwise.
 */
int32_t CRingBuffer_disable(CRingBuffer rb);

/**
 * Enables a previously disabled buffer.
 * After this call 'CRingBuffer_read' or 'CRingBuffer_write' functions operate normally.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, 0 otherwise.
 */
int32_t CRingBuffer_enable(CRingBuffer rb);

/**
 * Checks if the buffer is currently enabled.
 * Buffer may be disabled/enabled via 'CRingBuffer_disable'/'CRingBuffer_enable' functions respectively.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, 1 if the buffer is enabled, 0 if it's disabled.
 */
int32_t CRingBuffer_isEnabled(CRingBuffer rb);

/**
 * Purge existing data from the buffer.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, 0 otherwise.
 */
int32_t CRingBuffer_clear(CRingBuffer rb);

/**
 * Checks if the ring buffer is full.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, 1 if the buffer is full 0 otherwise.
 */
int32_t CRingBuffer_isFull(const CRingBuffer rb);

/**
 * Checks if the ring buffer is empty.
 *
 * @param[in] rb Valid ring buffer handle.
 * @return -1 on failure, 1 if the buffer is empty 0 otherwise.
 */
int32_t CRingBuffer_isEmpty(const CRingBuffer rb);

#ifdef __cplusplus
}
#endif

#endif /* CONCURRENTRINGBUFFER_H_ */
