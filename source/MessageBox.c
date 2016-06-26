/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/MessageBox.h"
#include "rb/ConcurrentRingBuffer.h"
#include "rb/Common.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define MESSAGE_BOX_MAGIC ( 0xAAF345BD )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    uint32_t magic;
    int32_t messageSize;
    int32_t capacity;
    CRingBufferHandle buffer;
} MessageBoxContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static MessageBoxContext* MessageBoxPriv_getContext(MessageBoxHandle handle);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

MessageBoxHandle MessageBox_new(int32_t messageSize, int32_t capacity) {
    MessageBoxContext* mb = (MessageBoxContext*) calloc(1,
            sizeof(MessageBoxContext));

    mb->magic = MESSAGE_BOX_MAGIC;
    mb->buffer = CRingBuffer_new(capacity * messageSize);
    mb->messageSize = messageSize;
    mb->capacity = capacity;

    if(mb->buffer == NULL) {
        free(mb);
        return NULL;
    }

    return mb;
}

int32_t MessageBox_free(MessageBoxHandle* handle) {
    MessageBoxContext* mb = MessageBoxPriv_getContext(*handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t rc = CRingBuffer_free(&mb->buffer);
    if(rc != RB_OK) {
        return rc;
    }

    free(mb);
    *handle = NULL;

    return RB_OK;
}

int32_t MessageBox_read(MessageBoxHandle handle, void* message) {
    return MessageBox_readTimed(handle, message, RB_WAIT_INFINITE);

}

int32_t MessageBox_readTimed(MessageBoxHandle handle, void* message, int32_t timeoutMs){
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t res = CRingBuffer_readTimed(mb->buffer, (uint8_t*) message,
            mb->messageSize, eREAD_BLOCK_FULL, timeoutMs);

    if(res != mb->messageSize && CRingBuffer_isEnabled(mb->buffer) == RB_FALSE) {
        return RB_DISABLED;
    }

    return res == mb->messageSize ? RB_OK : RB_ERROR;
}

int32_t MessageBox_write(MessageBoxHandle handle, const void* message) {
    return MessageBox_writeTimed(handle, message, RB_WAIT_INFINITE);
}

int32_t MessageBox_writeTimed(MessageBoxHandle handle, const void* message, int32_t timeoutMs){
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }
    int32_t res = CRingBuffer_writeTimed(mb->buffer, (const uint8_t*) message,
            mb->messageSize, eWRITE_BLOCK_FULL, timeoutMs);

    if(res != mb->messageSize && CRingBuffer_isEnabled(mb->buffer) == RB_FALSE) {
        return RB_DISABLED;
    }

    return res == mb->messageSize ? RB_OK : RB_ERROR;
}


int32_t MessageBox_getNumMessages(MessageBoxHandle handle) {
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t res = CRingBuffer_getBytesUsed(mb->buffer);

    if(res < 0) {
        return RB_ERROR;
    } else {
        return res / mb->messageSize;
    }
}

int32_t MessageBox_getCapacity(MessageBoxHandle handle){
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }


    return mb->capacity;
}

int32_t MessageBox_disable(MessageBoxHandle handle) {
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    return CRingBuffer_disable(mb->buffer);
}

int32_t MessageBox_enable(MessageBoxHandle handle) {
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    return CRingBuffer_enable(mb->buffer);
}

int32_t MessageBox_resize(MessageBoxHandle handle, uint32_t capacity){
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    mb->capacity = capacity;

    return CRingBuffer_resize(mb->buffer, mb->capacity * mb->messageSize);
}

MessageBoxContext* MessageBoxPriv_getContext(MessageBoxHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    MessageBoxContext* mb = (MessageBoxContext*) handle;
    if(mb->magic != MESSAGE_BOX_MAGIC) {
        return NULL;
    }

    return mb;
}
