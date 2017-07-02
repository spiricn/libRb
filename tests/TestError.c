/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/Log.h>
#include <rb/ConcurrentRingBuffer.h>
#include <rb/Utils.h>

#include <stdlib.h>
#include <pthread.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestError"

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    int32_t wrongMagic;
} InvalidStruct;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static void* testThread(void* arg);

static int32_t causeError();

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testError() {
    int rc;
    pthread_t testThreadId;
    void* threadRc;

    if (!RB_CHECK_VERSION) {
        RBLE("Invalid binary version");
        return -1;
    }

    // Cause an error on the main thread
    rc = causeError();
    if (rc != 0) {
        RBLE("causeError failed");
        return -1;
    }

    // Cause an error on another thread (previously caused error message shouldn't exist there)
    pthread_create(&testThreadId, NULL, testThread, NULL);
    pthread_join(testThreadId, &threadRc);

    if ((intptr_t) threadRc != 0) {
        RBLE("testThread failed");
        return -1;
    }

    return 0;
}

void* testThread(void* arg) {
    RB_UNUSED(arg);

    return (void*) (intptr_t) causeError();
}

int32_t causeError() {
    if (Rb_getLastErrorCode() != RB_OK || Rb_getLastErrorMessage()) {
        RBLE("Error code or message already set: code=%d message='%s'",
                Rb_getLastErrorCode(), Rb_getLastErrorMessage());
        return -1;
    }

    InvalidStruct wrongHandle = { 12345 };

    int32_t rc = Rb_CRingBuffer_isFull((Rb_CRingBufferHandle) &wrongHandle);

    if (rc == RB_OK) {
        RBLE("Function should have failed");
        return -1;
    }

    if (rc != Rb_getLastErrorCode()) {
        RBLE("Invalid last error code set: %d != %d", rc,
                Rb_getLastErrorCode());
        return -1;
    }

    const char* errorMessage = Rb_getLastErrorMessage();

    if (!errorMessage) {
        RBLE("Error message not set");
        return -1;
    }

    RBLI("Expected error: code=%d message='%s'", rc, errorMessage);

    return 0;
}
