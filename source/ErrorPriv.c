/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/priv/ErrorPriv.h"
#include "rb/Common.h"
#include "rb/Utils.h"

#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>

/********************************************************/
/*                 Local Module Variables (MODULE)      */
/********************************************************/

static __thread const char* gLastMessage = NULL;
static int32_t gLastErrorCode = RB_OK;

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

void Rb_errorPriv_setLastError(int32_t code, const char* fmt, ...){
    if(gLastMessage){
        RB_FREE(&gLastMessage);
    }

    va_list vl;
    va_start(vl, fmt);

    gLastMessage = Rb_Utils_printv(fmt, vl);

    va_end(vl);

    gLastErrorCode = code;
}

const char* Rb_getLastErrorMessage(){
    return gLastMessage;
}

int32_t Rb_getLastErrorCode(){
    return gLastErrorCode;
}
