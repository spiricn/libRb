#ifndef RB_ARRAY_H_
#define RB_ARRAY_H_

/********************************************************/
/*                 Includes                             */
/********************************************************/

#include "rb/Common.h"
#include <stdint.h>

/********************************************************/
/*                 Typedefs                             */
/********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Rb_StopwatchHandle;

/********************************************************/
/*                 Functions Declarations               */
/********************************************************/

/**
 * Creates new Stopwatch object
 *
 * @return Valid stopwatch handle on success, NULL otherwise.
 */
Rb_StopwatchHandle Rb_Stopwatch_new();

/**
 * Destroys Stopwatch object
 *
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Stopwatch_free(Rb_StopwatchHandle* handle);

/**
 * (Re)starts counting time.
 *
 * @return RB_OK on succes, negative value otherwise.
 */
int32_t Rb_Stopwatch_start(Rb_StopwatchHandle handle);

/**
 * Acquires current elapsed time since the last 'Rb_Stopwatch_start' call.
 *
 * @return Elapsed time in milliseconds on success, negative value otherwise.
 */
int64_t Rb_Stopwatch_elapsedMs(Rb_StopwatchHandle handle);

#ifdef __cplusplus
}
#endif

#endif
