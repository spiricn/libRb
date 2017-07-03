/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/RingBuffer.h>
#include <rb/Log.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestRingBUffer"

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testRingBuffer() {
    int32_t rc;
    int32_t i;

    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    const int32_t kCAPACITY = 10;

    uint8_t testData[kCAPACITY];
    uint8_t testOutData[kCAPACITY];

    // Initialize some data
    for(i = 0; i < kCAPACITY; i++) {
        testData[i] = i;
    }

    Rb_RingBufferHandle rb = Rb_RingBuffer_new(kCAPACITY);
    ASSERT_NOT_NULL(rb);

    ASSERT_EQUAL(kCAPACITY, Rb_RingBuffer_getCapacity(rb));
    ASSERT_EQUAL(kCAPACITY, Rb_RingBuffer_getBytesFree(rb));
    ASSERT_EQUAL(0, Rb_RingBuffer_getBytesUsed(rb));
    ASSERT(Rb_RingBuffer_isEmpty(rb));
    ASSERT(!Rb_RingBuffer_isFull(rb));

    // Write some data
    rc = Rb_RingBuffer_write(rb, testData, kCAPACITY);
    ASSERT_EQUAL(kCAPACITY, rc);

    // Make sure buffer is full
    ASSERT(!Rb_RingBuffer_isEmpty(rb));
    ASSERT(Rb_RingBuffer_isFull(rb));

    ASSERT_EQUAL(0, Rb_RingBuffer_getBytesFree(rb));
    ASSERT_EQUAL(kCAPACITY, Rb_RingBuffer_getBytesUsed(rb));

    // Read previously written data
    rc = Rb_RingBuffer_read(rb, testOutData, kCAPACITY);
    ASSERT_EQUAL(kCAPACITY, rc);

    // Test read data
    ASSERT_EQUAL(0, memcmp(testOutData, testData, kCAPACITY));

    // Make sure buffer is empty
    ASSERT(Rb_RingBuffer_isEmpty(rb));
    ASSERT(!Rb_RingBuffer_isFull(rb));

    // Write some data (fill it up)
    rc = Rb_RingBuffer_write(rb, testData, kCAPACITY);
    ASSERT_EQUAL(rc, kCAPACITY);

    const int32_t kNEW_CAPACITY = kCAPACITY * 2;

    rc = Rb_RingBuffer_resize(rb, kNEW_CAPACITY);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(kNEW_CAPACITY, Rb_RingBuffer_getCapacity(rb));

    uint8_t largeOutData[kNEW_CAPACITY];
    uint8_t largeTestData[kNEW_CAPACITY];
    for(i = 0; i < kNEW_CAPACITY; i++) {
        largeTestData[i] = i % 0xFF;
    }

    rc = Rb_RingBuffer_write(rb, largeTestData, kNEW_CAPACITY);
    ASSERT_EQUAL(kNEW_CAPACITY, rc);

    rc = Rb_RingBuffer_read(rb, largeOutData, kNEW_CAPACITY);
    ASSERT_EQUAL(kNEW_CAPACITY, rc);

    for(i = 0; i < kNEW_CAPACITY; i++) {
        ASSERT_EQUAL(i % 0xFF, largeOutData[i]);
    }

    rc = Rb_RingBuffer_free(&rb);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_NULL(rb);

    return 0;
}
