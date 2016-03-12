#ifndef MESSAGEBOX_H_
#define MESSAGEBOX_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* MessageBoxHandle;

/*
 * Creates new message box object
 *
 * @param[in] messageSize Size of single message
 * @return MessageBox object on sucess, NULL on failure
 */
MessageBoxHandle MessageBox_new(int32_t messageSize, int32_t capacity);

/**
 * Deallocate a MessageBox, and, as a side effect, set the pointer to NULL.
 *
 * @param[in,out] handle Pointer to a message box handle.
 * @return Negative value on failure, RB_OK on success.
 */
int32_t MessageBox_free(MessageBoxHandle* handle);

/**
 * Reads a single message.
 *
 * @param[in] handle Valid message box handle
 * @param[out] message Memory where read message will be stored
 * @return Negative value on failure, RB_OK on success
 */
int32_t MessageBox_read(MessageBoxHandle handle, void* message);

/**
 * Writes a single message.
 *
 * @param[in] handle Valid message box handle
 * @param[in] message Message memory
 * @return Negative value on failure, RB_OK on success
 */
int32_t MessageBox_write(MessageBoxHandle handle, const void* message);

/**
 * Acquires the total number of available messages
 *
 * @param[in] handle Valid message box handle
 * @return Negative value on failure, number of available messages otherwise
 */
int32_t MessageBox_getNumMessages(MessageBoxHandle handle);

/**
 * Disables underyling buffers and unblocks all pending operations. All following calls to message box will fail without blocking.
 *
 * @param[in] handle Valid message box handle
 * @return Negative value on failure, RB_OK otherwise
 */
int32_t MessageBox_disable(MessageBoxHandle handle);

/**
 * Enables a message box disabled via *MessageBox_disable*
 *
 * @param[in] handle Valid message box handle
 * @return Negative value on failure, RB_OK otherwise
 */
int32_t MessageBox_enable(MessageBoxHandle handle);

#ifdef __cplusplus
}
#endif

#endif

