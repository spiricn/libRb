/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "Stopwatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define STOPWATCH_MAGIC ( 0x1111AB34 )

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

static StopwatchContext* StopwatchPriv_getContext(StopwatchHandle handle);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/


StopwatchHandle Stopwatch_new(){
    StopwatchContext* sw = (StopwatchContext*)calloc(1, sizeof(StopwatchContext));

    sw->magic = STOPWATCH_MAGIC;

    return (StopwatchHandle)sw;
}

int32_t Stopwatch_free(StopwatchHandle* handle){
    StopwatchContext* sw = StopwatchPriv_getContext(*handle);
    if(sw == NULL) {
        return RB_INVALID_ARG;
    }

    free(sw);
    *handle = NULL;

    return RB_OK;
}

int32_t Stopwatch_start(StopwatchHandle handle){
    StopwatchContext* sw = StopwatchPriv_getContext(handle);
    if(sw == NULL) {
        return -1;
    }

    clock_gettime(CLOCK_REALTIME, &sw->time);

    return 0;
}

int64_t Stopwatch_elapsedMs(StopwatchHandle handle){
    StopwatchContext* sw = StopwatchPriv_getContext(handle);
    if(sw == NULL) {
        return -1;
    }

    struct timespec currTime;
    clock_gettime(CLOCK_REALTIME, &currTime);

    int64_t elapsedMs = 0;

    elapsedMs += ( currTime.tv_sec - sw->time.tv_sec ) * 1000;
    elapsedMs += ( currTime.tv_nsec - sw->time.tv_nsec ) / 1000000;

    return elapsedMs;
}

StopwatchContext* StopwatchPriv_getContext(StopwatchHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    StopwatchContext* sw = (StopwatchContext*)handle;
    if(sw->magic != STOPWATCH_MAGIC) {
        return NULL;
    }

    return sw;
}
