/*******************************************************/
/*              Includes                               */
/*******************************************************/

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

    Rb_StopwatchHandle sw = Rb_Stopwatch_new();
    if (sw == NULL) {
        RBLE("Rb_Stopwatch_new failed");
        return -1;
    }

    rc = Rb_Stopwatch_start(sw);
    if (rc != RB_OK) {
        RBLE("Rb_Stopwatch_start failed");
        return -1;
    }

    RBLD("Sleeping ..");
    usleep(SLEEP_PERIOD_MS * 1000);

    int64_t elapsedMs = Rb_Stopwatch_elapsedMs(sw);

    int64_t delta = elapsedMs - SLEEP_PERIOD_MS;

    // Take absolute value
    delta = delta < 0 ? -delta : delta;

    if (delta > ALLOWED_DELTA) {
        RBLE("Allowed delta exceeded: %lld", delta);
        return -1;
    }

    rc = Rb_Stopwatch_free(&sw);
    if (rc != RB_OK || sw != NULL) {
        RBLE("Rb_Stopwatch_free failed");
        return -1;
    }

    return 0;
}

