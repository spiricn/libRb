/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Timer.h"
#include "rb/Common.h"
#include "rb/Utils.h"
#include "rb/priv/ErrorPriv.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define TIMER_MAGIC ( 0x6632BC4F )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    uint32_t magic;
    bool running;
    Rb_TimerMode mode;
    uint64_t periodMs;
    Rb_TimerCallbackFnc fnc;
    void* userData;
    pthread_cond_t cv;
    pthread_mutex_t mutex;
    pthread_t thread;
} TimerContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static TimerContext* TimerPriv_getContext(Rb_TimerHandle handle);

static void* TimerPriv_waitThread(void* arg);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_TimerHandle Rb_Timer_new() {
    TimerContext* timer = (TimerContext*) RB_CALLOC(sizeof(TimerContext));

    timer->magic = TIMER_MAGIC;

    pthread_mutex_init(&timer->mutex, NULL);
    pthread_cond_init(&timer->cv, NULL);

    return (Rb_TimerHandle) timer;
}

int32_t Rb_Timer_free(Rb_TimerHandle* handle) {
    TimerContext* timer = TimerPriv_getContext(*handle);
    if(timer == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    if(timer->running){
        RB_ERRC(RB_ERROR, "Timer running, call stop first");
    }

    pthread_mutex_destroy(&timer->mutex);
    pthread_cond_destroy(&timer->cv);

    RB_FREE(&timer);
    *handle = NULL;

    return RB_OK;
}

int32_t Rb_Timer_start(Rb_TimerHandle handle, uint64_t periodMs,
        Rb_TimerMode mode, Rb_TimerCallbackFnc fnc, void* userData) {
    TimerContext* timer = TimerPriv_getContext(handle);
    if(timer == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    if(timer->running){
        RB_ERRC(RB_ERROR, "Timer already running");
    }

    timer->running = true;
    timer->mode = mode;
    timer->periodMs = periodMs;
    timer->fnc = fnc;
    timer->userData = userData;

    pthread_create(&timer->thread, NULL, TimerPriv_waitThread, (void*)timer);

    return RB_OK;
}

int32_t Rb_Timer_stop(Rb_TimerHandle handle) {
    TimerContext* timer = TimerPriv_getContext(handle);
    if(timer == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    if(!timer->running){
        RB_ERRC(RB_ERROR, "Timer not running");
    }

    pthread_mutex_lock(&timer->mutex);

    pthread_cond_broadcast(&timer->cv);

    timer->running = false;

    pthread_mutex_unlock(&timer->mutex);

    void* vrc;
    pthread_join(timer->thread, &vrc);

    return (intptr_t)vrc;
}

void* TimerPriv_waitThread(void* arg){
    TimerContext* timer = (TimerContext*)arg;
    int32_t rc;

    struct timespec time;

    while(true){
        pthread_mutex_lock(&timer->mutex);

        if(!timer->running){
            pthread_mutex_unlock(&timer->mutex);
            break;
        }

        Rb_Utils_getOffsetTime(&time, timer->periodMs);
        rc = pthread_cond_timedwait(&timer->cv, &timer->mutex, &time);

        pthread_mutex_unlock(&timer->mutex);

        if(rc == ETIMEDOUT){
            timer->fnc((Rb_TimerHandle)timer, timer->userData);
        }

        if(timer->mode == eRB_TIMER_MODE_ONE_SHOT){
            break;
        }
    }

    return (void*)RB_OK;
}

TimerContext* TimerPriv_getContext(Rb_TimerHandle handle) {
    if(handle == NULL) {
        return NULL ;
    }

    TimerContext* timer = (TimerContext*) handle;
    if(timer->magic != TIMER_MAGIC) {
        return NULL ;
    }

    return timer;
}
