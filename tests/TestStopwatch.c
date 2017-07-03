/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/Stopwatch.h>
#include <rb/Log.h>

#include <unistd.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define SLEEP_PERIOD_MS ( 1500 )
#define ALLOWED_DELTA ( 10 )

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestStopwatch"

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testStopwatch() {
    int32_t rc;
    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    Rb_StopwatchHandle sw = Rb_Stopwatch_new();
    ASSERT_NOT_NULL(sw);

    rc = Rb_Stopwatch_start(sw);
    ASSERT_EQUAL(RB_OK, rc);

    RBLD("Sleeping ..");
    usleep(SLEEP_PERIOD_MS * 1000);

    int64_t elapsedMs = Rb_Stopwatch_elapsedMs(sw);

    int64_t delta = elapsedMs - SLEEP_PERIOD_MS;

    // Take absolute value
    delta = delta < 0 ? -delta : delta;

    ASSERT(delta <= ALLOWED_DELTA);

    rc = Rb_Stopwatch_free(&sw);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(sw, NULL);

    return 0;
}

