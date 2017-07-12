/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/ConcurrentRingBuffer.h>
#include <rb/Log.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define TIMEOUT_MS ( 500 )

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestCRingBuffer"

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testCRingBuffer() {
    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    int32_t rc;
    int32_t bytesWritten;
    int32_t bytesRead;
    int32_t i;
    const int32_t kCAPACITY = 10;

    uint8_t testData[kCAPACITY];
    uint8_t testOutData[kCAPACITY];

    // Initialize some data
    for(i = 0; i < kCAPACITY; i++) {
        testData[i] = i;
    }

    Rb_CRingBufferHandle rb = Rb_CRingBuffer_new(kCAPACITY);
    ASSERT_NOT_NULL(rb);

    ASSERT_EQUAL_INT32(kCAPACITY, Rb_CRingBuffer_getCapacity(rb));
    ASSERT_EQUAL_INT32(kCAPACITY, Rb_CRingBuffer_getBytesFree(rb));
    ASSERT_EQUAL_INT32(0, Rb_CRingBuffer_getBytesUsed(rb));
    ASSERT(Rb_CRingBuffer_isEmpty(rb));
    ASSERT(!Rb_CRingBuffer_isFull(rb));

    // Timed read (expected timeout since empty)
    rc = Rb_CRingBuffer_readTimed(rb, testOutData, kCAPACITY,
            eCRB_READ_MODE_ALL, TIMEOUT_MS, &bytesRead);
    ASSERT_EQUAL_INT32(RB_TIMEOUT, rc);
    ASSERT_EQUAL_INT32(0, bytesRead);

    // Write
    rc = Rb_CRingBuffer_write(rb, testData, kCAPACITY, eCRB_WRITE_MODE_ALL, &bytesWritten);
    ASSERT_EQUAL_INT32(RB_OK, rc);
    ASSERT_EQUAL_INT32(kCAPACITY, bytesWritten);

    // Make sure buffer is full
    ASSERT(!Rb_CRingBuffer_isEmpty(rb));
    ASSERT(Rb_CRingBuffer_isFull(rb));
    ASSERT_EQUAL_INT32(0, Rb_CRingBuffer_getBytesFree(rb));
    ASSERT_EQUAL_INT32(kCAPACITY, Rb_CRingBuffer_getBytesUsed(rb));

    // Write timeout test
    rc = Rb_CRingBuffer_writeTimed(rb, testData, kCAPACITY,
            eCRB_WRITE_MODE_ALL, TIMEOUT_MS, &bytesWritten);
    ASSERT_EQUAL_INT32(RB_TIMEOUT, rc);
    ASSERT_EQUAL_INT32(0, bytesWritten);

    rc = Rb_CRingBuffer_read(rb, testOutData, kCAPACITY, eCRB_READ_MODE_ALL, &bytesRead);
    ASSERT_EQUAL_INT32(RB_OK, rc);
    ASSERT_EQUAL_INT32(kCAPACITY, bytesRead);

    // Make sure buffer is empty
    ASSERT(Rb_CRingBuffer_isEmpty(rb));
    ASSERT(!Rb_CRingBuffer_isFull(rb));

    // Test read data
    ASSERT_EQUAL_BFR(testOutData, testData, kCAPACITY);

    rc = Rb_CRingBuffer_free(&rb);
    ASSERT_EQUAL_INT32(RB_OK, rc);
    ASSERT_NULL(rb);

    return 0;
}
