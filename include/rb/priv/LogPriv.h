#ifndef RB_LOG_PRIV_H_
#define RB_LOG_PRIV_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/
#include "rb/Log.h"

#include <stdint.h>

/********************************************************/
/*                 Typedefs                             */
/********************************************************/

typedef enum {
    eRB_LOG_COMPONENT_TAG,
    eRB_LOG_COMPONENT_TIMESTAMP,
    eRB_LOG_COMPONENT_MESSAGE,
    eRB_LOG_COMPONENT_LEVEL,
    eRB_LOG_COMPONENT_FILE,
    eRB_LOG_COMPONENT_LINE,
    eRB_LOG_COMPONENT_FUNCTION,
    eRB_LOG_COMPONENT_TEXT,
} Rb_LogComponentType;

typedef struct {
    Rb_LogComponentType type;
    char* value;
} Rb_LogComponent;

typedef struct {
    Rb_LogComponent* components;
    uint32_t numComponents;
} Rb_CompiledFormat;

typedef struct {
    uint32_t start;
    uint32_t end;
} Rb_FormatMatch;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

int32_t Rb_logPriv_compileFormat(const char* source,
        Rb_CompiledFormat* compiledFormat);

int32_t Rb_logPriv_parseFormat(const char* source,
        Rb_FormatMatch** outMatches, uint32_t* outNumMatches);

char* Rb_logPriv_formatMessage(const Rb_MessageInfo* message, const Rb_CompiledFormat* format);

#endif
