/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/BlockingQueue.h>
#include <rb/Log.h>
#include <rb/Utils.h>

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

    if (!RB_CHECK_VERSION) {
        RBLE("Invalid binary version");
        return -1;
    }

    // Create blocking queue
    Rb_BlockingQueueHandle bq = Rb_BlockingQueue_new(sizeof(Message),
    NUM_MESSAGES);
    if (!bq) {
        RBLE("Rb_BlockingQueue_new failed: %s / %d", Rb_getLastErrorMessage(),
                Rb_getLastErrorCode());
        return -1;
    }

    // Check capacity
    if (Rb_BlockingQueue_getCapacity(bq) != NUM_MESSAGES) {
        RBLE("Rb_BlockingQueue_getCapacity failed: %s / %d",
                Rb_getLastErrorMessage(), Rb_getLastErrorCode());
        return -1;
    }

    // Check if it's empty
    if (Rb_BlockingQueue_getNumMessages(bq)) {
        RBLE("Rb_BlockingQueue_getNumMessages failed: %s / %d",
                Rb_getLastErrorMessage(), Rb_getLastErrorCode());
        return -1;
    }

    Message msgIn = { 42 };
    Message msgOut;

    // Write message
    rc = Rb_BlockingQueue_put(bq, &msgIn);
    if (rc != RB_OK) {
        RBLE("Rb_BlockingQueue_put failed: %s / %d", Rb_getLastErrorMessage(),
                Rb_getLastErrorCode());
        return -1;
    }

    if (Rb_BlockingQueue_getNumMessages(bq) != 1) {
        RBLE("Rb_BlockingQueue_getNumMessages failed: %s / %d",
                Rb_getLastErrorMessage(), Rb_getLastErrorCode());
        return -1;
    }

    // Peek message
    rc = Rb_BlockingQueue_peek(bq, &msgOut);
    if (rc != RB_OK) {
        RBLE("Rb_BlockingQueue_peek failed: %s / %d", Rb_getLastErrorMessage(),
                Rb_getLastErrorCode());
        return -1;
    }

    if (Rb_BlockingQueue_getNumMessages(bq) != 1) {
        RBLE("Rb_BlockingQueue_getNumMessages failed: %s / %d",
                Rb_getLastErrorMessage(), Rb_getLastErrorCode());
        return -1;
    }

    // Check data
    if (memcmp(&msgIn, &msgOut, sizeof(Message))) {
        RBLE("Invalid data");
        return -1;
    }

    // Read message
    rc = Rb_BlockingQueue_get(bq, &msgOut);
    if (rc != RB_OK) {
        RBLE("Rb_BlockingQueue_get failed: %s / %d", Rb_getLastErrorMessage(),
                Rb_getLastErrorCode());
        return -1;
    }

    // Check data
    if (memcmp(&msgIn, &msgOut, sizeof(Message))) {
        RBLE("Invalid data");
        return -1;
    }

    if (Rb_BlockingQueue_getNumMessages(bq) != 0) {
        RBLE("Invalid number of messages");
        return -1;
    }

    // Reader thread will finish first
    pthread_t readerTid;
    pthread_create(&readerTid, NULL, readerThread, (void*) bq);

    pthread_t writerTid;
    pthread_create(&writerTid, NULL, writerThread, (void*) bq);

    void* vrc = NULL;

    pthread_join(readerTid, &vrc);
    if ((intptr_t) vrc != 0) {
        RBLE("Reader thread failed");
        return -1;
    }

    // In order for writer thread to stop blocking, we need to clear the queue
    if (Rb_BlockingQueue_clear(bq) != RB_OK) {
        RBLE("Rb_BlockingQueue_clear failed");
        return -1;
    }

    RBLD("Cleared queue");

    while (!Rb_BlockingQueue_isFull(bq)) {
        usleep(1000);
    }

    RBLD("Queue filled, resizing ..");
    rc = Rb_BlockingQueue_resize(bq, Rb_BlockingQueue_getCapacity(bq) + 64);
    if (rc != RB_OK) {
        RBLE("Rb_BlockingQueue_resize failed");
        return -1;
    }

    pthread_join(writerTid, &vrc);
    if ((intptr_t) vrc != 0) {
        RBLE("Writer thread failed");
        return -1;
    }

    // Destroy blocking queue
    rc = Rb_BlockingQueue_free(&bq);
    if (rc != RB_OK && bq) {
        RBLE("Rb_BlockingQueue_free failed: %s / %d", Rb_getLastErrorMessage(),
                Rb_getLastErrorCode());
        return -1;
    }

    return 0;
}

void* writerThread(void* arg) {
    RBLD("Write thread started");

    Rb_BlockingQueueHandle bq = (Rb_BlockingQueueHandle) arg;

    int32_t i;
    int32_t j;
    int32_t rc;

    for (j = 0; j < 3; j++) {
        int32_t numMessages = 0;
        if (j == 0) {
            numMessages = NUM_TEST_MESSAGES;
        } else if (j == 1) {
            numMessages = Rb_BlockingQueue_getCapacity(bq);
        } else if (j == 2) {
            numMessages = Rb_BlockingQueue_getCapacity(bq) + 3;
        }

        RBLD("Started write iteration: %d", numMessages);

        for (i = 0; i < numMessages; i++) {
            Message msg;
            memset(&msg, 0x00, sizeof(Message));

            msg.num = i;

            rc = Rb_BlockingQueue_put(bq, &msg);
            if (rc != RB_OK) {
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

    for (i = 0; i < NUM_TEST_MESSAGES; i++) {
        Message msg;
        memset(&msg, 0x00, sizeof(Message));

        rc = Rb_BlockingQueue_get(bq, &msg);
        if (rc != RB_OK) {
            RBLE("Rb_BlockingQueue_get failed: %s / %d",
                    Rb_getLastErrorMessage(), Rb_getLastErrorCode());
            return (void*) -1;
        }

        if (msg.num != i) {
            RBLE("Invalid data: %d != %d", msg.num, i);
            return (void*) -1;
        }

    }
    RBLD("Read thread stopped");

    return NULL;
}
