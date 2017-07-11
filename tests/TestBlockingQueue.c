/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/BlockingQueue.h>
#include <rb/Log.h>
#include <rb/Utils.h>

#include <unistd.h>
#include <pthread.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestBlockingQueue"

#define NUM_MESSAGES ( 32 )
#define NUM_TEST_MESSAGES ( 1024 * 10 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    int32_t num;
} Message;

typedef struct {
    int32_t numMessages;
    Rb_BlockingQueueHandle bq;
} WriterThreadArg;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static void* writerThread(void* arg);

static void* readerThread(void* arg);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testBlockingQueue() {
    int32_t rc;

    ASSERT_EQUAL_INT32(RB_TRUE, RB_CHECK_VERSION);

    // Create blocking queue
    Rb_BlockingQueueHandle bq = Rb_BlockingQueue_new(sizeof(Message),
    NUM_MESSAGES);
    ASSERT_NOT_NULL(bq);

    // Check capacity
    ASSERT_EQUAL_INT32(NUM_MESSAGES, Rb_BlockingQueue_getCapacity(bq));

    // Check if it's empty
    ASSERT_EQUAL_INT32(0, Rb_BlockingQueue_getNumMessages(bq));

    Message msgIn = { 42 };
    Message msgOut;

    // Test timeut
    rc = Rb_BlockingQueue_getTimed(bq, &msgOut, 50);
    ASSERT_EQUAL_INT32(RB_TIMEOUT, rc);

    rc = Rb_BlockingQueue_peekTimed(bq, &msgOut, 50);
    ASSERT_EQUAL_INT32(RB_TIMEOUT, rc);

    // Write message
    rc = Rb_BlockingQueue_put(bq, &msgIn);
    ASSERT_EQUAL_INT32(RB_OK, rc);
    ASSERT_EQUAL_INT32(1, Rb_BlockingQueue_getNumMessages(bq));

    // Peek message
    rc = Rb_BlockingQueue_peek(bq, &msgOut);
    ASSERT_EQUAL_INT32(RB_OK, rc);
    ASSERT_EQUAL_INT32(1, Rb_BlockingQueue_getNumMessages(bq));

    // Check data
    ASSERT_EQUAL_INT32(0, memcmp(&msgIn, &msgOut, sizeof(Message)));

    // Read message
    rc = Rb_BlockingQueue_get(bq, &msgOut);
    ASSERT_EQUAL_INT32(RB_OK, rc);
    ASSERT_EQUAL_INT32(0, Rb_BlockingQueue_getNumMessages(bq));

    // Check data
    ASSERT_EQUAL_INT32(0, memcmp(&msgIn, &msgOut, sizeof(Message)));

    // Read NUM_TEST_MESSAGES from the queue
    pthread_t readerTid;
    ASSERT_EQUAL_INT32(0, pthread_create(&readerTid, NULL, readerThread, (void*) bq));

    // Write NUM_TEST_MESSAGES messages to the queue
    pthread_t writerTid;
    WriterThreadArg warg;
    warg.bq = bq;
    warg.numMessages = NUM_TEST_MESSAGES;
    ASSERT_EQUAL_INT32(0, pthread_create(&writerTid, NULL, writerThread, (void*) &warg));

    void* vrc = NULL;

    // Wait for reader & writer to finish
    pthread_join(readerTid, &vrc);
    ASSERT_EQUAL_INT32(0, (intptr_t ) vrc);

    pthread_join(writerTid, &vrc);
    ASSERT_EQUAL_INT32(0, (intptr_t ) vrc);

    ASSERT_EQUAL_INT32(0, Rb_BlockingQueue_getNumMessages(bq));

    // Fill up the queue with max messages
    warg.bq = bq;
    warg.numMessages = Rb_BlockingQueue_getCapacity(bq);
    ASSERT_EQUAL_INT32(0, pthread_create(&writerTid, NULL, writerThread, (void*) &warg));

    pthread_join(writerTid, &vrc);
    ASSERT_EQUAL_INT32(0, (intptr_t ) vrc);
    ASSERT(Rb_BlockingQueue_isFull(bq));

    // Since the queue is full, this call should timeout
    rc = Rb_BlockingQueue_putTimed(bq, &msgIn, 50);
    ASSERT_EQUAL_INT32(RB_TIMEOUT, rc);

    // Attempt to fill again (should block)
    warg.bq = bq;
    warg.numMessages = Rb_BlockingQueue_getCapacity(bq);
    ASSERT_EQUAL_INT32(0, pthread_create(&writerTid, NULL, writerThread, (void*) &warg));

    usleep(1000);

    // Clear the queue, making room for new messages
    rc = Rb_BlockingQueue_clear(bq);
    ASSERT_EQUAL_INT32(RB_OK, rc);

    // Writer thread should no unblock
    pthread_join(writerTid, &vrc);
    ASSERT_EQUAL_INT32(0, (intptr_t ) vrc);
    // Queue should be full
    ASSERT(Rb_BlockingQueue_isFull(bq));

    // Attempt to fill again (should block)
    warg.bq = bq;
    warg.numMessages = Rb_BlockingQueue_getCapacity(bq);
    ASSERT_EQUAL_INT32(0, pthread_create(&writerTid, NULL, writerThread, (void*) &warg));

    usleep(1000);

    // Resize it, making room for new messages
    RBLD("Queue filled, resizing ..");
    int32_t newCapacity = Rb_BlockingQueue_getCapacity(bq)  * 2;
    rc = Rb_BlockingQueue_resize(bq, newCapacity);
    ASSERT_EQUAL_INT32(RB_OK, rc);
    ASSERT_EQUAL_INT32(newCapacity, Rb_BlockingQueue_getCapacity(bq));

    pthread_join(writerTid, &vrc);
    ASSERT_EQUAL_INT32(0, (intptr_t ) vrc);
    // Queue should be full
    ASSERT(Rb_BlockingQueue_isFull(bq));

    // Destroy blocking queue
    rc = Rb_BlockingQueue_free(&bq);
    ASSERT_EQUAL_INT32(RB_OK, rc);
    ASSERT_NULL(bq);

    return 0;
}

void* writerThread(void* arg) {
    WriterThreadArg* warg = (WriterThreadArg*) arg;

    int32_t i;
    int32_t rc;

    RBLD("Write thread started, writing %d messages", warg->numMessages);

    for(i = 0; i <warg->numMessages; i++) {
        Message msg;
        memset(&msg, 0x00, sizeof(Message));

        msg.num = i;

        rc = Rb_BlockingQueue_put(warg->bq, &msg);
        if(rc != RB_OK) {
            RBLE("Rb_BlockingQueue_put failed: %s / %d / %d",
                    Rb_getLastErrorMessage(), Rb_getLastErrorCode(), rc);
            return (void*) -1;
        }
    }

    RBLD("Write thread stopped");

    return NULL;
}

void* readerThread(void* arg) {
    RBLD("Read thread stopped");

    int32_t i;
    int32_t rc;

    Rb_BlockingQueueHandle bq = (Rb_BlockingQueueHandle) arg;

    for(i = 0; i < NUM_TEST_MESSAGES; i++) {
        Message msg;
        memset(&msg, 0x00, sizeof(Message));

        rc = Rb_BlockingQueue_get(bq, &msg);
        if(rc != RB_OK) {
            RBLE("Rb_BlockingQueue_get failed: %s / %d",
                    Rb_getLastErrorMessage(), Rb_getLastErrorCode());
            return (void*) -1;
        }

        if(msg.num != i) {
            RBLE("Invalid data: %d != %d", msg.num, i);
            return (void*) -1;
        }

    }
    RBLD("Read thread stopped");

    return NULL;
}
