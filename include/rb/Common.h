
#ifndef RB_COMMON_H_
#define RB_COMMON_H_

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
#define RB_VERSION_MINOR ( 2 )
#define RB_VERSION_PATCH ( 0 )
#define RB_VERSION_NUMBER(major, minor, patch) ((uint64_t)(  ( ((uint64_t)(major) & 0xFFFF) << 48 ) | ( ((uint64_t)(minor) & 0xFFFF) << 32 ) | ((uint64_t)(patch) & 0xFFFFFFFF ) ) )
#define RB_CHECK_VERSION ( Rb_getVersion() == RB_VERSION_NUMBER((uint64_t)RB_VERSION_MAJOR, (uint64_t)RB_VERSION_MINOR, (uint64_t)RB_VERSION_PATCH) )
#define RB_OK ( 0 )
#define RB_TRUE ( 1 )
#define RB_FALSE ( 0 )

#define RB_ERROR_BASE ( -1000 )

#define RB_ERROR ( RB_ERROR_BASE - 1 )
#define RB_DISABLED ( RB_ERROR_BASE - 2 )
#define RB_INVALID_ARG ( RB_ERROR_BASE - 3 )
#define RB_TIMEOUT ( RB_ERROR_BASE - 4 )
#define RB_NOT_IMPLEMENTED ( RB_ERROR_BASE - 5 )

#define RB_WAIT_INFINITE ( INT_MIN )

#define RB_UNUSED(var) do{ (void*)&var; }while(0)

#define RB_STRING_SMALL ( 256 )
#define RB_STRING_LARGE ( 1024 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef enum{
    eRB_VAR_TYPE_INT32,
    eRB_VAR_TYPE_INT64,
    eRB_VAR_TYPE_FLOAT,
    eRB_VAR_TYPE_STRING,
    eRB_VAR_TYPE_BLOB
} Rb_VariantType;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

uint64_t Rb_getVersion();

#ifdef __cplusplus
}
#endif

#endif
