/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Utils.h"

#include <stdarg.h>
#include <stdlib.h>

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

char* Rb_Utils_print(const char* fmt, ...){
    va_list vl;

    va_start(vl, fmt);

    char* str = Rb_Utils_printv(fmt, vl);

    va_end(vl);

    return str;
}

char* Rb_Utils_printv(const char* fmt, va_list vl){
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
