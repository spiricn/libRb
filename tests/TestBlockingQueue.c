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

    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    // Create blocking queue
    Rb_BlockingQueueHandle bq = Rb_BlockingQueue_new(sizeof(Message),
    NUM_MESSAGES);
    ASSERT_NOT_NULL(bq);

    // Check capacity
    ASSERT_EQUAL(NUM_MESSAGES, Rb_BlockingQueue_getCapacity(bq));

    // Check if it's empty
    ASSERT_EQUAL(0, Rb_BlockingQueue_getNumMessages(bq));

    Message msgIn = { 42 };
    Message msgOut;

    // Write message
    rc = Rb_BlockingQueue_put(bq, &msgIn);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(1, Rb_BlockingQueue_getNumMessages(bq));

    // Peek message
    rc = Rb_BlockingQueue_peek(bq, &msgOut);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(1, Rb_BlockingQueue_getNumMessages(bq));

    // Check data
    ASSERT_EQUAL(0, memcmp(&msgIn, &msgOut, sizeof(Message)));

    // Read message
    rc = Rb_BlockingQueue_get(bq, &msgOut);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(0, Rb_BlockingQueue_getNumMessages(bq));

    // Check data
    ASSERT_EQUAL(0, memcmp(&msgIn, &msgOut, sizeof(Message)));

    // Reader thread will finish first
    pthread_t readerTid;
    pthread_create(&readerTid, NULL, readerThread, (void*) bq);

    pthread_t writerTid;
    pthread_create(&writerTid, NULL, writerThread, (void*) bq);

    void* vrc = NULL;

    pthread_join(readerTid, &vrc);
    ASSERT_EQUAL(0, (intptr_t ) vrc);

    // In order for writer thread to stop blocking, we need to clear the queue
    rc = Rb_BlockingQueue_clear(bq);
    ASSERT_EQUAL(RB_OK, rc);

    RBLD("Cleared queue");

    while(!Rb_BlockingQueue_isFull(bq)) {
        usleep(1000);
    }

    RBLD("Queue filled, resizing ..");
    rc = Rb_BlockingQueue_resize(bq, Rb_BlockingQueue_getCapacity(bq) + 64);
    ASSERT_EQUAL(RB_OK, rc);

    pthread_join(writerTid, &vrc);
    ASSERT_EQUAL(0, (intptr_t ) vrc);

    // Destroy blocking queue
    rc = Rb_BlockingQueue_free(&bq);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_NULL(bq);

    return 0;
}

void* writerThread(void* arg) {
    RBLD("Write thread started");

    Rb_BlockingQueueHandle bq = (Rb_BlockingQueueHandle) arg;

    int32_t i;
    int32_t j;
    int32_t rc;

    for(j = 0; j < 3; j++) {
        int32_t numMessages = 0;
        if(j == 0) {
            numMessages = NUM_TEST_MESSAGES;
        } else if(j == 1) {
            numMessages = Rb_BlockingQueue_getCapacity(bq);
        } else if(j == 2) {
            numMessages = Rb_BlockingQueue_getCapacity(bq) + 3;
        }

        RBLD("Started write iteration: %d", numMessages);

        for(i = 0; i < numMessages; i++) {
            Message msg;
            memset(&msg, 0x00, sizeof(Message));

            msg.num = i;

            rc = Rb_BlockingQueue_put(bq, &msg);
            if(rc != RB_OK) {
                RBLE("Rb_BlockingQueue_put failed: %s / %d",
                        Rb_getLastErrorMessage(), Rb_getLastErrorCode());
                return (void*) -1;
            }
        }

        RBLD("Stopped write iteration");
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
