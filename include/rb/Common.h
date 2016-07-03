#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <limits.h>
#include <stdint.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define RB_VERSION_MAJOR ( 1 )
#define RB_VERSION_MINOR ( 0 )
#define RB_VERSION_PATCH ( 0 )
#define RB_VERSION_NUMBER(major, minor, patch)  ((uint64_t)( ( ((major) & 0xFFFF) << 48 ) | ( ((minor) & 0xFFFF) << 32 ) | ( (patch) & 0xFFFFFFFF ) ))
#define RB_CHECK_VERSION ( Rb_getVersion() == RB_VERSION_NUMBER((uint64_t)RB_VERSION_MAJOR, (uint64_t)RB_VERSION_MINOR, (uint64_t)RB_VERSION_PATCH) )
#define RB_OK ( 0 )
#define RB_TRUE ( 1 )
#define RB_FALSE ( 0 )

#define RB_ERROR_BASE ( -1000 )

#define RB_ERROR ( RB_ERROR_BASE - 1 )
#define RB_DISABLED ( RB_ERROR_BASE - 2 )
#define RB_INVALID_ARG ( RB_ERROR_BASE - 3 )
#define RB_TIMEOUT ( RB_ERROR_BASE - 4 )

#define RB_WAIT_INFINITE ( INT_MIN )

#define RB_UNUSED(var) do{ (void*)&var; }while(0)

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

uint64_t Rb_getVersion();

#ifdef __cplusplus
}
#endif

#endif
