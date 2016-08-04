#ifndef RB_TIMER_H_
#define RB_TIMER_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Common.h"

#include <stdint.h>


/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Rb_TimerHandle;

typedef enum {
    eRB_TIMER_MODE_ONE_SHOT,
    eRB_TIMER_MODE_PERIODIC,
} Rb_TimerMode;

typedef void (*Rb_TimerCallbackFnc)(Rb_TimerHandle handle, void* userData);

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/**
 * Creates a new timer.
 *
 * @return Timer handle on success, NULL otherwise.
 */
Rb_TimerHandle Rb_Timer_new();

/**
 * Frees existing timer.
 *
 * @param[in] handle Pointer to a valid timer handle.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Timer_free(Rb_TimerHandle* handle);

/**
 * Starts a timer. If already started, timer must be first stopped via Rb_Timer_stop.
 *
 * @param[in] handle Valid timer handle.
 * @param[in] periodMs Timer period in milliseconds.
 * @param[in] mode Timer mode.
 * @param[in] fnc Timer callback function.
 * @param[in] userData Timer callback function user data.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Timer_start(Rb_TimerHandle handle, uint64_t periodMs, Rb_TimerMode mode, Rb_TimerCallbackFnc fnc, void* userData);

/**
 * Stops a running timer.
 *
 * @param[in] handle Valid timer handle
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Timer_stop(Rb_TimerHandle handle);


#ifdef __cplusplus
}
#endif

#endif
