/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "MessageBox.h"
#include "ConcurrentRingBuffer.h"
#include "Common.h"

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
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t res = CRingBuffer_read(mb->buffer, (uint8_t*) message,
            mb->messageSize, eREAD_BLOCK_FULL);

    if(res != mb->messageSize && CRingBuffer_isEnabled(mb->buffer) == RB_FALSE) {
        return RB_DISABLED;
    }

    return res == mb->messageSize ? RB_OK : RB_ERROR;
}

int32_t MessageBox_write(MessageBoxHandle handle, const void* message) {
    MessageBoxContext* mb = MessageBoxPriv_getContext(handle);
    if(mb == NULL) {
        return RB_INVALID_ARG;
    }
    int32_t res = CRingBuffer_write(mb->buffer, (const uint8_t*) message,
            mb->messageSize, eWRITE_BLOCK_FULL);

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
