/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/IOStream.h"
#include "rb/Utils.h"
#include "rb/Common.h"
#include "rb/priv/ErrorPriv.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int32_t Rb_IOStream_print(const Rb_IOStream* stream, const char* fmt, ...){
    va_list vl;

    va_start(vl, fmt);

    char* str = Rb_Utils_printv(fmt, vl);

    va_end(vl);

    if(stream->api.write(stream->handle, str, strlen(str) + 1) != (int32_t)strlen(str) + 1){
        RB_FREE(&str);

        RB_ERRC(RB_ERROR, "Failed writing to stream");
    }

    RB_FREE(&str);

    return RB_OK;
}
