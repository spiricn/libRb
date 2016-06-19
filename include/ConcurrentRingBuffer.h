#ifndef CONCURRENTRINGBUFFER_H_
#define CONCURRENTRINGBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "Common.h"

#include <stdint.h>
#include <pthread.h>

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef enum {
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
} CRingBuffer_ReadMode;

typedef enum {
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

typedef void* CRingBufferHandle;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/**
 * Creates new concurrent ring buffer object
 *
 * @param[in] size Buffer capacity
 * @return Buffer object on sucess, NULL on failure
 */
CRingBufferHandle CRingBuffer_new(uint32_t size);

/**
 * Creates ring buffer from an already allocated memory block (may be shared between processes).
 *
 * @param[in] memory Memory block where the buffer was allocated.
 * @param[in] size Size of the provided memory block.
 * @param[in] init If set to 1 initializes the memory (creates a new buffer object). If set to 0 assumes the buffer was already created.
 * @return Buffer object on sucess, NULL on failure
 */
CRingBufferHandle CRingBuffer_fromSharedMemory(void* memory, uint32_t size,
        int init);

/**
 * Frees a buffer object created via 'CRingBuffer_new' or 'CRingBuffer_fromSharedMemory' functions.
 *
 * @param[in,out] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_OK on success.
 */
int32_t CRingBuffer_free(CRingBufferHandle* handle);

/**
 * Reads data from the buffer. May block depending on the read mode.
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] data Destination buffer.
 * @param[in] size Size of the destination buffer.
 * @param[in] mode Mode which decides the behavior of the function call. See 'CRingBuffer_ReadMode' enumeration for more info.
 * @return Negative value on failure, number of bytes read otherwise.
 */
int32_t CRingBuffer_read(CRingBufferHandle handle, uint8_t* data, uint32_t size, CRingBuffer_ReadMode mode);

/**
 * Reads data from the buffer. May block depending on the read mode.
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] data Destination buffer.
 * @param[in] size Size of the destination buffer.
 * @param[in] mode Mode which decides the behavior of the function call. See 'CRingBuffer_ReadMode' enumeration for more info.
 * @param[in] timeoutMs Time in milliseconds after which the function times out and exists with a failure.
 * @return Negative value on failure, number of bytes read otherwise.
 */
int32_t CRingBuffer_readTimed(CRingBufferHandle handle, uint8_t* data, uint32_t size, CRingBuffer_ReadMode mode, int64_t timeoutMs);

/**
 * Writes data to the buffer. May block depending on the write mode.
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] data Source buffer.
 * @param[in] size Size of the source buffer.
 * @param[in] mode Mode which decides the behavior of the function call. See 'CRingBuffer_WriteMode' enumeration for more info.
 * @return Negative value on failure, number of bytes written otherwise.
 */
int32_t CRingBuffer_write(CRingBufferHandle handle, const uint8_t* data,
        uint32_t size, CRingBuffer_WriteMode mode);

/**
 * Writes data to the buffer. May block depending on the write mode.
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] data Source buffer.
 * @param[in] size Size of the source buffer.
 * @param[in] mode Mode which decides the behavior of the function call. See 'CRingBuffer_WriteMode' enumeration for more info.
 * @param[in] timeoutMs Time in milliseconds after which the function times out and exists with a failure.
 * @return Negative value on failure, number of bytes written otherwise.
 */
int32_t CRingBuffer_writeTimed(CRingBufferHandle handle, const uint8_t* data,
        uint32_t size, CRingBuffer_WriteMode mode, int64_t timeoutMs);

/**
 * Gets the number of bytes currently contained in the buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, number of bytes contained otherwise.
 */
int32_t CRingBuffer_getBytesUsed(CRingBufferHandle handle);

/**
 * Gets the number of free space left in the buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, number of bytes free otherwise.
 */
int32_t CRingBuffer_getBytesFree(CRingBufferHandle handle);

/**
 * Gets the total capacity of the buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, ring buffer capacity otherwise.
 */
int32_t CRingBuffer_getCapacity(CRingBufferHandle handle);

/**
 * Disables the ring buffer. All currently blocking operations (e.g. 'CRingBuffer_read' or 'CRingBuffer_write' calls) will return immediately.
 * After this call any further 'CRingBuffer_read' or 'CRingBuffer_write' calls will return 0 without blocking.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_OK otherwise.
 */
int32_t CRingBuffer_disable(CRingBufferHandle handle);

/**
 * Enables a previously disabled buffer.
 * After this call 'CRingBuffer_read' or 'CRingBuffer_write' functions operate normally.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_OK otherwise.
 */
int32_t CRingBuffer_enable(CRingBufferHandle handle);

/**
 * Checks if the buffer is currently enabled.
 * Buffer may be disabled/enabled via 'CRingBuffer_disable'/'CRingBuffer_enable' functions respectively.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_TRUE if the buffer is enabled, RB_FALSE otherwise
 */
int32_t CRingBuffer_isEnabled(CRingBufferHandle handle);

/**
 * Purge existing data from the buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_OK otherwise.
 */
int32_t CRingBuffer_clear(CRingBufferHandle handle);

/**
 * Checks if the ring buffer is full.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, 1 if the buffer is full 0 otherwise.
 */
int32_t CRingBuffer_isFull(CRingBufferHandle handle);

/**
 * Checks if the ring buffer is empty.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, 1 if the buffer is empty 0 otherwise.
 */
int32_t CRingBuffer_isEmpty(CRingBufferHandle handle);

#ifdef __cplusplus
}
#endif

#endif
