/* @file MessageBox.c
 * @author Nikola Spiric <nikola.spiric@rt-rk.com>
 */

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "MessageBox.h"
#include "ConcurrentRingBuffer.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define MESSAGE_BOX_MAGIC ( 0xAAF345BD )

#define VALID_HANDLE(handle) ( (handle) != NULL && (handle)->magic == MESSAGE_BOX_MAGIC )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

struct MessageBox_t {
    uint32_t magic;
    int32_t messageSize;
    CRingBuffer buffer;
};
// </ConcurrentRingBuffer_t>

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

MessageBox MessageBox_new(int32_t messageSize, int32_t capacity) {
    MessageBox mb = (MessageBox)malloc(sizeof(struct MessageBox_t));

    memset(mb, 0x00, sizeof(struct MessageBox_t));

    mb->magic = MESSAGE_BOX_MAGIC;

    mb->buffer = CRingBuffer_new(capacity * messageSize);

    mb->messageSize = messageSize;

    return mb;
}

int32_t MessageBox_free(MessageBox* mb) {
    if(mb == NULL || !VALID_HANDLE(*mb)) {
        return -1;
    }

    CRingBuffer_free(&(*mb)->buffer);

    free(*mb);

    *mb = NULL;

    return 0;
}

int32_t MessageBox_read(MessageBox mb, void* message) {
    if(!VALID_HANDLE(mb)) {
        return -1;
    }

    int32_t res = CRingBuffer_read(mb->buffer, (uint8_t*)message, mb->messageSize, eREAD_BLOCK_FULL);

    return res == mb->messageSize ? 0 : -1;
}

int32_t MessageBox_write(MessageBox mb, const void* message) {
    if(!VALID_HANDLE(mb)) {
        return -1;
    }

    int32_t res = CRingBuffer_write(mb->buffer, (const uint8_t*)message, mb->messageSize, eWRITE_BLOCK_FULL);

    return res == mb->messageSize ? 0 : -1;
}

int32_t MessageBox_getNumMessages(MessageBox mb) {
    if(!VALID_HANDLE(mb)) {
        return -1;
    }

    int32_t res = CRingBuffer_getBytesUsed(mb->buffer);

    if(res == -1) {
        return -1;
    } else {
        return res / mb->messageSize;
    }
}

int32_t MessageBox_disable(MessageBox mb) {
    if(!VALID_HANDLE(mb)) {
        return -1;
    }

    return CRingBuffer_disable(mb->buffer);
}

int32_t MessageBox_enable(MessageBox mb) {
    if(!VALID_HANDLE(mb)) {
        return -1;
    }

    return CRingBuffer_enable(mb->buffer);
}
