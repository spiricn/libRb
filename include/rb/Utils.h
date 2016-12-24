#ifndef RB_UTILS_H_
#define RB_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <stdint.h>
#include <stdarg.h>
#include <time.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/


/*******************************************************/
/*              Typedefs                               */
/*******************************************************/


/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

char* Rb_Utils_print(const char* fmt, ...);

char* Rb_Utils_printv(const char* fmt, va_list vl);

void Rb_Utils_getOffsetTime(struct timespec* time, int64_t offsetMs);

void Rb_Utils_growAppend(char** base, uint32_t baseSize, uint32_t* newSize, const char* str);


#ifdef __cplusplus
}
#endif

#endif
