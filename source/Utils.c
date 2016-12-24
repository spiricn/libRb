/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

void Rb_Utils_growAppend(char** base, uint32_t baseSize, uint32_t* newSize,
        char* str) {
    if (strlen(*base) + strlen(str) + 1 > baseSize) {
        // Not big enough to fit
        *newSize = strlen(*base) + strlen(str) + 1;
        *base = realloc(*base, *newSize);
    }
    strcat(*base, str);
}

char* Rb_Utils_print(const char* fmt, ...) {
    va_list vl;

    va_start(vl, fmt);

    char* str = Rb_Utils_printv(fmt, vl);

    va_end(vl);

    return str;
}

char* Rb_Utils_printv(const char* fmt, va_list vl) {
    va_list vlSize;

    va_copy(vlSize, vl);
    int32_t size = vsnprintf(NULL, 0, fmt, vlSize) + 1;
    va_end(vlSize);

    va_list vlPrint;

    va_copy(vlPrint, vl);
    char* str = (char*) calloc(1, size);

    vsnprintf(str, size, fmt, vl);
    va_end(vlPrint);

    return str;
}

void Rb_Utils_getOffsetTime(struct timespec* time, int64_t offsetMs) {
    clock_gettime(CLOCK_REALTIME, time);

    int64_t offsetNs = offsetMs * 1000000L;
    int64_t ns = time->tv_nsec + offsetNs;

    time->tv_nsec = ns % 1000000000L;
    time->tv_sec = time->tv_sec + (ns / 1000000000L);
}
