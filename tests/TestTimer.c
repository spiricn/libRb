/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/Timer.h>
#include <rb/Log.h>

#include <semaphore.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestTimer"

#define TEST_WAIT_TIME_MS ( 1000 )
#define NUM_TEST_PERIODS ( 3 )

typedef struct {
    sem_t sem;
} TimerTest;

static void testTimerCallback(Rb_TimerHandle handle, void* userData);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testTimer() {
    int32_t rc;
    uint32_t i;

    TimerTest ctx;

    sem_init(&ctx.sem, 0, 0);

    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    Rb_TimerHandle timer = Rb_Timer_new();
    ASSERT_NOT_NULL(timer);

    // Test one shot
    rc = Rb_Timer_start(timer, TEST_WAIT_TIME_MS, eRB_TIMER_MODE_ONE_SHOT,
            testTimerCallback, &ctx);
    ASSERT_EQUAL(RB_OK, rc);

    sem_wait(&ctx.sem);

    rc = Rb_Timer_stop(timer);
    ASSERT_EQUAL(RB_OK, rc);

    // Test periodic
    rc = Rb_Timer_start(timer, TEST_WAIT_TIME_MS, eRB_TIMER_MODE_PERIODIC,
            testTimerCallback, &ctx);
    ASSERT_EQUAL(RB_OK, rc);

    for (i = 0; i < NUM_TEST_PERIODS; i++) {
        sem_wait(&ctx.sem);
    }

    rc = Rb_Timer_stop(timer);
    ASSERT_EQUAL(RB_OK, rc);

    rc = Rb_Timer_free(&timer);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(timer, NULL);

    return 0;
}

void testTimerCallback(Rb_TimerHandle handle, void* userData) {
    RB_UNUSED(handle);

    TimerTest* ctx = (TimerTest*) userData;

    sem_post(&ctx->sem);

    RBLI("Timer callback");
}
