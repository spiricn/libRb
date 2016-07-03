#ifndef MESSAGEBOX_H_
#define MESSAGEBOX_H_

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

typedef void* Rb_MessageBoxHandle;

/********************************************************/
/*                 Functions Declarations               */
/********************************************************/

/*
 * Creates new message box object
 *
 * @param[in] messageSize Size of single message
 * @return MessageBox object on sucess, NULL on failure
 */
Rb_MessageBoxHandle Rb_MessageBox_new(int32_t messageSize, int32_t capacity);

/**
 * Deallocate a MessageBox, and, as a side effect, set the pointer to NULL.
 *
 * @param[in,out] handle Pointer to a message box handle.
 * @return Negative value on failure, RB_OK on success.
 */
int32_t Rb_MessageBox_free(Rb_MessageBoxHandle* handle);

/**
 * Reads a single message.
 *
 * @param[in] handle Valid message box handle
 * @param[out] message Memory where read message will be stored
 * @return Negative value on failure, RB_OK on success
 */
int32_t Rb_MessageBox_read(Rb_MessageBoxHandle handle, void* message);

/**
 * Reads a single message.
 *
 * @param[in] handle Valid message box handle
 * @param[in] timeoutMs Time in milliseconds after which the function times out and exists with a failure.
 * @param[out] message Memory where read message will be stored
 * @return Negative value on failure, RB_OK on success
 */
int32_t Rb_MessageBox_readTimed(Rb_MessageBoxHandle handle, void* message, int32_t timeoutMs);

/**
 * Writes a single message.
 *
 * @param[in] handle Valid message box handle
 * @param[in] message Message memory
 * @return Negative value on failure, RB_OK on success
 */
int32_t Rb_MessageBox_write(Rb_MessageBoxHandle handle, const void* message);

/**
 * Writes a single message.
 *
 * @param[in] handle Valid message box handle
 * @param[in] message Message memory
 * @param[in] timeoutMs Time in milliseconds after which the function times out and exists with a failure.
 * @return Negative value on failure, RB_OK on success
 */
int32_t Rb_MessageBox_writeTimed(Rb_MessageBoxHandle handle, const void* message, int32_t timeoutMs);

/**
 * Acquires the total number of available messages
 *
 * @param[in] handle Valid message box handle
 * @return Negative value on failure, number of available messages otherwise
 */
int32_t Rb_MessageBox_getNumMessages(Rb_MessageBoxHandle handle);

/**
 * Acquires message box capacity
 *
 * @param[in] handle Valid message box handle
 * @return Negative value on failure, message box capacity otherwise
 */
int32_t Rb_MessageBox_getCapacity(Rb_MessageBoxHandle handle);

/**
 * Disables underyling buffers and unblocks all pending operations. All following calls to message box will fail without blocking.
 *
 * @param[in] handle Valid message box handle
 * @return Negative value on failure, RB_OK otherwise
 */
int32_t Rb_MessageBox_disable(Rb_MessageBoxHandle handle);

/**
 * Enables a message box disabled via *MessageBox_disable*
 *
 * @param[in] handle Valid message box handle
 * @return Negative value on failure, RB_OK otherwise
 */
int32_t Rb_MessageBox_enable(Rb_MessageBoxHandle handle);

/**
 * Resizes the message box capacity.
 *
 * @param[in] handle Valid message box handle.
 * @param[in] New message box capacity.
 * @return Negative value on failure, RB_OK otherwise
 */
int32_t Rb_MessageBox_resize(Rb_MessageBoxHandle handle, uint32_t capacity);

#ifdef __cplusplus
}
#endif

#endif

