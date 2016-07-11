#ifndef RB_PREFS_BACKEND_H_
#define RB_PREFS_BACKEND_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Prefs.h"

#include <stdint.h>


/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

int32_t Rb_PrefsBackendSave(Rb_PrefsHandle handle, const Rb_IOStream* stream);

int32_t Rb_PrefsBackendLoad(Rb_PrefsHandle handle, const Rb_IOStream* stream);

#ifdef __cplusplus
}
#endif

#endif
