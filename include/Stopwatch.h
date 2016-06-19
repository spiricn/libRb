#ifndef ARRAY_H_
#define ARRAY_H_

/********************************************************/
/*                 Includes                             */
/********************************************************/

#include "Common.h"
#include <stdint.h>

/********************************************************/
/*                 Typedefs                             */
/********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void* StopwatchHandle;

/********************************************************/
/*                 Functions Declarations               */
/********************************************************/

/**
 * Creates new Stopwatch object
 *
 * @return Valid stopwatch handle on success, NULL otherwise.
 */
StopwatchHandle Stopwatch_new();

/**
 * Destroys Stopwatch object
 *
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Stopwatch_free(StopwatchHandle* handle);

/**
 * (Re)starts counting time.
 *
 * @return RB_OK on succes, negative value otherwise.
 */
int32_t Stopwatch_start(StopwatchHandle handle);

/**
 * Acquires current elapsed time since the last 'Stopwatch_start' call.
 *
 * @return Elapsed time in milliseconds on success, negative value otherwise.
 */
int64_t Stopwatch_elapsedMs(StopwatchHandle handle);

#ifdef __cplusplus
}
#endif

#endif
