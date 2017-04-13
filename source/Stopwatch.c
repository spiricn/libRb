/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Stopwatch.h"
#include "rb/Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define STOPWATCH_MAGIC ( 0x1111AB34 )
#define MS_IN_S ( 1000 )
#define NS_IN_S ( 1000000 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    int32_t magic;
    struct timespec time;
} StopwatchContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static StopwatchContext* StopwatchPriv_getContext(Rb_StopwatchHandle handle);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/


Rb_StopwatchHandle Rb_Stopwatch_new(){
    StopwatchContext* sw = (StopwatchContext*)RB_CALLOC(sizeof(StopwatchContext));

    sw->magic = STOPWATCH_MAGIC;

    return (Rb_StopwatchHandle)sw;
}

int32_t Rb_Stopwatch_free(Rb_StopwatchHandle* handle){
    StopwatchContext* sw = StopwatchPriv_getContext(*handle);
    if(sw == NULL) {
        return RB_INVALID_ARG;
    }

    RB_FREE(&sw);
    *handle = NULL;

    return RB_OK;
}

int32_t Rb_Stopwatch_start(Rb_StopwatchHandle handle){
    StopwatchContext* sw = StopwatchPriv_getContext(handle);
    if(sw == NULL) {
        return -1;
    }

    clock_gettime(CLOCK_REALTIME, &sw->time);

    return 0;
}

int64_t Rb_Stopwatch_elapsedMs(Rb_StopwatchHandle handle){
    StopwatchContext* sw = StopwatchPriv_getContext(handle);
    if(sw == NULL) {
        return -1;
    }

    struct timespec currTime;
    clock_gettime(CLOCK_REALTIME, &currTime);

    int64_t elapsedMs = 0;

    elapsedMs += ( currTime.tv_sec - sw->time.tv_sec ) * MS_IN_S;
    elapsedMs += ( currTime.tv_nsec - sw->time.tv_nsec ) / NS_IN_S;

    return elapsedMs;
}

StopwatchContext* StopwatchPriv_getContext(Rb_StopwatchHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    StopwatchContext* sw = (StopwatchContext*)handle;
    if(sw->magic != STOPWATCH_MAGIC) {
        return NULL;
    }

    return sw;
}
