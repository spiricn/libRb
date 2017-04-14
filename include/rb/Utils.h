#ifndef RB_UTILS_H_
#define RB_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/Common.h>

#include <stdint.h>
#include <stdarg.h>
#include <time.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define RB_MALLOC(size) Rb_malloc(size)
#define RB_CALLOC(size) Rb_calloc(size)
#define RB_FREE(ptr) Rb_free((void**)ptr)
#define RB_REALLOC(ptr, newSize) Rb_realloc((void*)ptr, newSize)

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

void* Rb_malloc(int32_t size);

int32_t Rb_free(void** ptr);

void* Rb_calloc(int32_t size);

void* Rb_realloc(void* ptr, int32_t size);

const char* Rb_getLastErrorMessage();

int32_t Rb_getLastErrorCode();

#ifdef __cplusplus
}
#endif

#endif
