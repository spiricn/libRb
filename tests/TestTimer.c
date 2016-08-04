/*******************************************************/
/*              Includes                               */
/*******************************************************/

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

    if(!RB_CHECK_VERSION){
        RBLE("Invalid binary version");
        return -1;
    }

    Rb_TimerHandle timer = Rb_Timer_new();
    if(!timer){
        RBLE("Rb_Timer_new failed");
        return -1;
    }

    // Test one shot
    rc = Rb_Timer_start(timer, TEST_WAIT_TIME_MS, eRB_TIMER_MODE_ONE_SHOT, testTimerCallback, &ctx);
    if(rc != RB_OK){
        RBLE("Rb_Timer_start failed");
        return -1;
    }

    sem_wait(&ctx.sem);

    rc = Rb_Timer_stop(timer);
    if(rc != RB_OK){
        RBLE("Rb_Timer_stop failed");
        return -1;
    }

    // Test periodic
    rc = Rb_Timer_start(timer, TEST_WAIT_TIME_MS, eRB_TIMER_MODE_PERIODIC, testTimerCallback, &ctx);
    if(rc != RB_OK){
        RBLE("Rb_Timer_start failed");
        return -1;
    }

    for(i=0; i<NUM_TEST_PERIODS; i++){
        sem_wait(&ctx.sem);
    }

    rc = Rb_Timer_stop(timer);
    if(rc != RB_OK){
        RBLE("Rb_Timer_stop failed");
        return -1;
    }

    rc = Rb_Timer_free(&timer);
    if(rc != RB_OK || timer){
        RBLE("Rb_Timer_free failed");
        return -1;
    }

    return 0;
}

void testTimerCallback(Rb_TimerHandle handle, void* userData){
    RB_UNUSED(handle);

    TimerTest* ctx = (TimerTest*)userData;

    sem_post(&ctx->sem);

    RBLI("Timer callback");
}
