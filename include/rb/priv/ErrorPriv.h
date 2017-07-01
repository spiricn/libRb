#ifndef RB_ERROR_PRIV_H_
#define RB_ERROR_PRIV_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Log.h"

#include <stdint.h>

/********************************************************/
/*                 Defines                              */
/********************************************************/

#define RB_ERR(fmt, ...) \
    do{ \
        Rb_errorPriv_setLastError(RB_ERROR, "[%s:%d]: " fmt, RB_FILENAME, __LINE__, ## __VA_ARGS__); \
    } while(0)

#define RB_ERRC(code, fmt, ...) \
    do{ \
        Rb_errorPriv_setLastError(code,  "[%s:%d]: " fmt, RB_FILENAME, __LINE__, ## __VA_ARGS__); \
        return code; \
    } while(0)

/********************************************************/
/*                 Functions Declarations               */
/********************************************************/

void Rb_errorPriv_setLastError(int32_t code, const char* message, ...);

#endif
