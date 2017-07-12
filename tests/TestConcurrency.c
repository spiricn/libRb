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

    // Create the consumer thread
    pthread_t consumerThread;
    ASSERT_EQUAL_INT32(0,
            pthread_create(&consumerThread, NULL, consumer, (void* ) rb));

    // Write data into the buffer
    uint32_t bytesWritten = 0;
    while(bytesWritten != NUM_TEST_DATA) {
        static const uint32_t kBFR_SIZE = 37;

        // Create some data
        uint8_t data[kBFR_SIZE];
        uint32_t i;
        for(i = 0; i < kBFR_SIZE; i++) {
            data[i] = (bytesWritten + i) % 0xFF;
        }

        uint32_t bytesLeft = NUM_TEST_DATA - bytesWritten;
        uint32_t toWrite = bytesLeft > kBFR_SIZE ? kBFR_SIZE : bytesLeft;

        // Write it
        int32_t outWritten;
        rc = Rb_CRingBuffer_write(rb, data, toWrite, eCRB_WRITE_MODE_ALL,
                &outWritten);
        ASSERT_EQUAL_INT32(RB_OK, rc);
        ASSERT_EQUAL_INT32(toWrite, outWritten);

        bytesWritten += rc;
    }

    RBLI("Producer thread done");

    void* vrc = NULL;
    ASSERT_EQUAL_INT32(0, pthread_join(consumerThread, &vrc));
    ASSERT_EQUAL_INT32(0, vrc);

    rc = Rb_CRingBuffer_free(&rb);
    ASSERT_EQUAL_INT32(RB_OK, rc);
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

        int32_t outRead = -1;
        int32_t rc = Rb_CRingBuffer_read(rb, bfr, toRead, eCRB_READ_MODE_ALL,
                &outRead);
        ASSERT_EQUAL_INT32_R(RB_OK, rc, (void* )-1);
        ASSERT_EQUAL_INT32_R(toRead, outRead, (void* )-1);

        int32_t i;
        for(i = 0; i < rc; i++) {
            ASSERT_EQUAL_INT32_R(bfr[i], (bytesRead + i) % 0xFF, (void* )-1);
        }

        bytesRead += rc;
    }

    RBLI("Consumer thread done");

    return (void*) 0;
}
