/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/ConcurrentRingBuffer.h>
#include <rb/Log.h>
#include <pthread.h>
#include <stdbool.h>

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "testConcurrency"

#define NUM_TEST_DATA ( 10 * 1024 )

static void* consumer(void* arg);

int testConcurrency() {
    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    int32_t rc;
    int32_t i;
    const int32_t kCAPACITY = 1024;

    Rb_CRingBufferHandle rb = Rb_CRingBuffer_new(kCAPACITY);
    ASSERT_NOT_NULL(rb);

    pthread_t consumerThread;
    pthread_create(&consumerThread, NULL, consumer, (void*) rb);

    uint32_t bytesWritten = 0;
    while(bytesWritten != NUM_TEST_DATA) {
        static const uint32_t kBFR_SIZE = 37;

        uint8_t data[kBFR_SIZE];

        uint32_t i;
        for(i = 0; i < kBFR_SIZE; i++) {
            data[i] = (bytesWritten + i) % 0xFF;
        }

        uint32_t bytesLeft = NUM_TEST_DATA - bytesWritten;
        uint32_t toWrite = bytesLeft > kBFR_SIZE ? kBFR_SIZE : bytesLeft;

        rc = Rb_CRingBuffer_write(rb, data, toWrite, eRB_WRITE_BLOCK_FULL);
        ASSERT_EQUAL(toWrite, rc);

        bytesWritten += rc;
    }

    RBLI("Producer thread done");

    void* vrc = NULL;
    pthread_join(consumerThread, &vrc);
    ASSERT_EQUAL(0, vrc);

    rc = Rb_CRingBuffer_free(&rb);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_NULL(rb);

    return 0;
}

void* consumer(void* arg) {
    Rb_CRingBufferHandle rb = (Rb_CRingBufferHandle) arg;

    static const uint32_t kBUFFER_SIZE = 17;
    uint8_t bfr[kBUFFER_SIZE];
    uint32_t bytesRead = 0;

    while(bytesRead != NUM_TEST_DATA) {
        uint32_t bytesLeft = NUM_TEST_DATA - bytesRead;
        uint32_t toRead = bytesLeft > kBUFFER_SIZE ? kBUFFER_SIZE : bytesLeft;

        int32_t rc = Rb_CRingBuffer_read(rb, bfr, toRead, eRB_READ_BLOCK_FULL);
        if(rc != (int32_t) toRead) {
            RBLE("Rb_CRingBuffer_read failed: %d", rc);
            return (void*) -1;
        }

        int32_t i;
        for(i = 0; i < rc; i++) {
            if(bfr[i] != (bytesRead + i) % 0xFF) {
                RBLE("Invalid data read: %d != %d", bfr[i], bytesRead + i);
                return (void*) -1;
            }
        }

        bytesRead += rc;
    }

    RBLI("Consumer thread done");

    return (void*) 0;
}
