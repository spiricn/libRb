#ifndef RB_CONCURRENT_RING_BUFFER_H_
#define RB_CONCURRENT_RING_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Common.h"

#include <stdint.h>
#include <pthread.h>

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef enum {
    eCRB_READ_MODE_INVALID,

    /**
     * Reads all the requested data. May block.
     */
    eCRB_READ_MODE_ALL,

    /**
     * Reads at least some of the requested data. May block.
     */
    eCRB_READ_MODE_SOME,
} Rb_CRingBuffer_ReadMode;

typedef enum {
    eCRB_WRITE_MODE_INVALID,

    /**
     * Writes all the data, without overwriting existing. May block.
     */
    eCRB_WRITE_MODE_ALL,

    /**
     * Writes all data possibly overwriting existing. Does not block.
     */
    eCRB_WRITE_MODE_OVERFLOW,

    /**
     * Writes at least some data, without overwriting existing. May block.
     */
    eCRB_WRITE_MODE_SOME
} Rb_CRingBuffer_WriteMode;

typedef void* Rb_CRingBufferHandle;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/**
 * Creates new concurrent ring buffer object
 *
 * @param[in] size Buffer capacity
 * @return Buffer object on sucess, NULL on failure
 */
Rb_CRingBufferHandle Rb_CRingBuffer_new(int32_t size);

/**
 * Creates ring buffer from an already allocated memory block (may be shared between processes).
 *
 * @param[in] memory Memory block where the buffer was allocated.
 * @param[in] size Size of the provided memory block.
 * @param[in] init If set to 1 initializes the memory (creates a new buffer object). If set to 0 assumes the buffer was already created.
 * @return Buffer object on sucess, NULL on failure
 */
Rb_CRingBufferHandle Rb_CRingBuffer_fromSharedMemory(void* memory, uint32_t size,
        int init);

/**
 * Frees a buffer object created via 'CRingBuffer_new' or 'CRingBuffer_fromSharedMemory' functions.
 *
 * @param[in,out] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_OK on success.
 */
int32_t Rb_CRingBuffer_free(Rb_CRingBufferHandle* handle);

/**
 * Reads data from the buffer. May block depending on the read mode.
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] data Destination buffer.
 * @param[in] size Size of the destination buffer.
 * @param[in] mode Mode which decides the behavior of the function call. See 'CRingBuffer_ReadMode' enumeration for more info.
 * @return
 *      - If mode is set to eCRB_READ_MODE_BLOCK_FULL:
 *          - If returnValue < size and Rb_CRingBuffer_isEnabled returns true, all data wasn't written due to buffer being disabled.
 *          - If returnValue == size, call succeeded.
 *          - Otherwise, an error occurred.
 *
 *      - If mode is set to eCRB_READ_MODE_SOME:
 *          - If returnValue <= 0, an error occurred.
 *          - Otherwise, the function call succeeded.
 */
int32_t Rb_CRingBuffer_read(Rb_CRingBufferHandle handle, uint8_t* data, int32_t size, Rb_CRingBuffer_ReadMode mode, int32_t* bytesRead);

/**
 * Reads data from the buffer. May block depending on the read mode.
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] data Destination buffer.
 * @param[in] size Size of the destination buffer.
 * @param[in] mode Mode which decides the behavior of the function call. See 'CRingBuffer_ReadMode' enumeration for more info.
 * @param[in] timeoutMs Time in milliseconds after which the function times out and exists with a failure.
 * @return
 *      - If mode is set to eCRB_READ_MODE_BLOCK_FULL:
 *          - If returnValue < size and Rb_CRingBuffer_isEnabled returns true, all data wasn't written due to buffer being disabled.
 *          - If returnValue < size, Rb_CRingBUffer_isEnabled returns false. and timeoutMs > 0, all data wasn't written due to timeout.
 *          - If returnValue == size, call succeeded.
 *          - Otherwise, an error occurred.
 *
 *      - If mode is set to eCRB_READ_MODE_SOME:
 *          - If returnValue <= 0, an error occurred.
 *          - Otherwise, the function call succeeded.
 */
int32_t Rb_CRingBuffer_readTimed(Rb_CRingBufferHandle handle, uint8_t* data, int32_t size, Rb_CRingBuffer_ReadMode mode, int64_t timeoutMs, int32_t* bytesRead);

/**
 * Writes data to the buffer. May block depending on the write mode.
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] data Source buffer.
 * @param[in] size Size of the source buffer.
 * @param[in] mode Mode which decides the behavior of the function call. See 'CRingBuffer_WriteMode' enumeration for more info.
 * @return Negative value on failure, number of bytes written otherwise.
 */
int32_t Rb_CRingBuffer_write(Rb_CRingBufferHandle handle, const uint8_t* data,
        int32_t size, Rb_CRingBuffer_WriteMode mode, int32_t* bytesWritten);

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
int32_t Rb_CRingBuffer_writeTimed(Rb_CRingBufferHandle handle, const uint8_t* data,
        int32_t size, Rb_CRingBuffer_WriteMode mode, int64_t timeoutMs, int32_t* bytesWritten);

/**
 * Gets the number of bytes currently contained in the buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, number of bytes contained otherwise.
 */
int32_t Rb_CRingBuffer_getBytesUsed(Rb_CRingBufferHandle handle);

/**
 * Gets the number of free space left in the buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, number of bytes free otherwise.
 */
int32_t Rb_CRingBuffer_getBytesFree(Rb_CRingBufferHandle handle);

/**
 * Gets the total capacity of the buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, ring buffer capacity otherwise.
 */
int32_t Rb_CRingBuffer_getCapacity(Rb_CRingBufferHandle handle);

/**
 * Disables the ring buffer. All currently blocking operations (e.g. 'CRingBuffer_read' or 'CRingBuffer_write' calls) will return immediately.
 * After this call any further 'CRingBuffer_read' or 'CRingBuffer_write' calls will return 0 without blocking.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_OK otherwise.
 */
int32_t Rb_CRingBuffer_disable(Rb_CRingBufferHandle handle);

/**
 * Enables a previously disabled buffer.
 * After this call 'CRingBuffer_read' or 'CRingBuffer_write' functions operate normally.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_OK otherwise.
 */
int32_t Rb_CRingBuffer_enable(Rb_CRingBufferHandle handle);

/**
 * Checks if the buffer is currently enabled.
 * Buffer may be disabled/enabled via 'CRingBuffer_disable'/'CRingBuffer_enable' functions respectively.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_TRUE if the buffer is enabled, RB_FALSE otherwise
 */
int32_t Rb_CRingBuffer_isEnabled(Rb_CRingBufferHandle handle);

/**
 * Purge existing data from the buffer.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, RB_OK otherwise.
 */
int32_t Rb_CRingBuffer_clear(Rb_CRingBufferHandle handle);

/**
 * Checks if the ring buffer is full.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, 1 if the buffer is full 0 otherwise.
 */
int32_t Rb_CRingBuffer_isFull(Rb_CRingBufferHandle handle);

/**
 * Checks if the ring buffer is empty.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, 1 if the buffer is empty 0 otherwise.
 */
int32_t Rb_CRingBuffer_isEmpty(Rb_CRingBufferHandle handle);

/**
 * Grows or shrinks ring buffers internal buffer
 *
 * @param[in] handle Valid ring buffer handle.
 * @param[in] capacity New ring buffer size.
 * @return Negative value on failure, RB_OK otherwise.
 */
int32_t Rb_CRingBuffer_resize(Rb_CRingBufferHandle handle, uint32_t capacity);

/**
 * Gets currently used space in percentage.
 *
 * @param[in] handle Valid ring buffer handle.
 * @return Negative value on failure, number of bytes free otherwise.
 */
float Rb_CRingBuffer_usedSpacePercentage(Rb_CRingBufferHandle handle);
#ifdef __cplusplus
}
#endif

#endif
