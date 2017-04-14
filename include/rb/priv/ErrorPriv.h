#ifndef RB_ERROR_PRIV_H_
#define RB_ERROR_PRIV_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <stdint.h>

/********************************************************/
/*                 Defines                              */
/********************************************************/

#define RB_ERR(fmt, ...) Rb_errorPriv_setLastError(RB_ERROR, fmt, # __VA_ARGS__)
#define RB_ERRC(code, fmt, ...) do{ Rb_errorPriv_setLastError(code, fmt, # __VA_ARGS__); return code; } while(0)

/********************************************************/
/*                 Functions Declarations               */
/********************************************************/

void Rb_errorPriv_setLastError(int32_t code, const char* message, ...);

#endif
