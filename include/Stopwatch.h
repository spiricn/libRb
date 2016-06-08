#ifndef ARRAY_H_
#define ARRAY_H_

#include "Common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* StopwatchHandle;

StopwatchHandle Stopwatch_new();

int32_t Stopwatch_free(StopwatchHandle* handle);

int32_t Stopwatch_start(StopwatchHandle handle);

int64_t Stopwatch_elapsedMs(StopwatchHandle handle);

#ifdef __cplusplus
}
#endif

#endif
