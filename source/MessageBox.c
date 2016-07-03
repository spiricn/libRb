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
    Rb_CRingBufferHandle buffer;
} MessageBoxContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static MessageBoxContext* MessageBoxPriv_getContext(Rb_MessageBoxHandle handle);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_MessageBoxHandle Rb_MessageBox_new(int32_t messageSize, int32_t capacity) {
    MessageBoxContext* mb = (MessageBoxContext*) calloc(1,
            sizeof(MessageBoxContext));

    mb->magic = MESSAGE_BOX_MAGIC;
    mb->buffer = Rb_CRingBuffer_new(capacity * messageSize);
    mb->messageSize = messageSize;
    mb->capacity = capacity;

    if(mb->buffer == NULL) {
        free(mb);
        return NULL;
    }

    return mb;
}

int32_t Rb_MessageBox_free(Rb_MessageBoxHandle* handle) {
    MessageBoxContext* mb = MessageBoxPriv_getContext(*handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t rc = Rb_CRingBuffer_free(&mb->buffer);
    if(rc != RB_OK) {
        return rc;
    }

    free(mb);
    *handle = NULL;

    return RB_OK;
}

int32_t Rb_MessageBox_read(Rb_MessageBoxHandle handle, void* message) {
    return Rb_MessageBox_readTimed(handle, message, RB_WAIT_INFINITE);
}

int32_t Rb_MessageBox_readTimed(Rb_MessageBoxHandle handle, void* message, int32_t timeoutMs){
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t res = Rb_CRingBuffer_readTimed(mb->buffer, (uint8_t*) message,
            mb->messageSize, eRB_READ_BLOCK_FULL, timeoutMs);

    if(res != mb->messageSize && Rb_CRingBuffer_isEnabled(mb->buffer) == RB_FALSE) {
        return RB_DISABLED;
    }

    return res == mb->messageSize ? RB_OK : RB_ERROR;
}

int32_t Rb_MessageBox_write(Rb_MessageBoxHandle handle, const void* message) {
    return Rb_MessageBox_writeTimed(handle, message, RB_WAIT_INFINITE);
}

int32_t Rb_MessageBox_writeTimed(Rb_MessageBoxHandle handle, const void* message, int32_t timeoutMs){
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }
    int32_t res = Rb_CRingBuffer_writeTimed(mb->buffer, (const uint8_t*) message,
            mb->messageSize, eRB_WRITE_BLOCK_FULL, timeoutMs);

    if(res != mb->messageSize && Rb_CRingBuffer_isEnabled(mb->buffer) == RB_FALSE) {
        return RB_DISABLED;
    }

    return res == mb->messageSize ? RB_OK : RB_ERROR;
}


int32_t Rb_MessageBox_getNumMessages(Rb_MessageBoxHandle handle) {
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t res = Rb_CRingBuffer_getBytesUsed(mb->buffer);

    if(res < 0) {
        return RB_ERROR;
    } else {
        return res / mb->messageSize;
    }
}

int32_t Rb_MessageBox_getCapacity(Rb_MessageBoxHandle handle){
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }


    return mb->capacity;
}

int32_t Rb_MessageBox_disable(Rb_MessageBoxHandle handle) {
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    return Rb_CRingBuffer_disable(mb->buffer);
}

int32_t Rb_MessageBox_enable(Rb_MessageBoxHandle handle) {
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    return Rb_CRingBuffer_enable(mb->buffer);
}

int32_t Rb_MessageBox_resize(Rb_MessageBoxHandle handle, uint32_t capacity){
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    mb->capacity = capacity;

    return Rb_CRingBuffer_resize(mb->buffer, mb->capacity * mb->messageSize);
}

MessageBoxContext* MessageBoxPriv_getContext(Rb_MessageBoxHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    MessageBoxContext* mb = (MessageBoxContext*) handle;
    if(mb->magic != MESSAGE_BOX_MAGIC) {
        return NULL;
    }

    return mb;
}
