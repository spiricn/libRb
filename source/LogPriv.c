/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/priv/LogPriv.h"
#include "rb/Common.h"
#include "rb/Log.h"

#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>

/********************************************************/
/*                 Defines                              */
/********************************************************/

#define COMP_NAME_TAG "TAG"
#define COMP_NAME_TIMESTAPM "TIMESTAMP"
#define COMP_NAME_MESSAGE "MESSAGE"
#define COMP_NAME_LEVEL "LEVEL"
#define COMP_NAME_FILE "FILE"
#define COMP_NAME_LINE "LINE"
#define COMP_NAME_FUNCTION "FUNCTION"
#define COMP_NAME_TID "TID"
#define COMP_NAME_PID "PID"
#define MAX_FORMAT_COMPONENTS ( 64 )

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int32_t Rb_logPriv_compileFormat(const char* source,
        Rb_CompiledFormat* compiledFormat) {
    int rc;

    Rb_FormatMatch* formatMatches = NULL;
    uint32_t numFormatMatches = 0;
    int32_t currFormatMatch = 0;

    rc = Rb_logPriv_parseFormat(source, &formatMatches, &numFormatMatches);
    if (rc != RB_OK) {
        return rc;
    }

    Rb_LogComponent logComponents[MAX_FORMAT_COMPONENTS];
    memset(&logComponents, 0x00,
            sizeof(Rb_LogComponent) * MAX_FORMAT_COMPONENTS);
    uint32_t numLogComponents = 0;

    Rb_FormatMatch* nextMatch = numFormatMatches ? &formatMatches[0] : NULL;

    int32_t currPos = 0;
    int32_t len = strlen(source);

    while (1) {
        int32_t end;
        if (nextMatch) {
            end = nextMatch->start;
        } else {
            end = strlen(source);
        }

        if (currPos == end) {
            break;
        }

        if (currPos < end) {
            // Text between next component
            int32_t componentLen = end - currPos;

            char* componentText = malloc(componentLen + 1);

            strncpy(componentText, source + currPos, componentLen);
            componentText[componentLen] = 0;

            Rb_LogComponent* logComponent = &logComponents[numLogComponents++];
            logComponent->type = eRB_LOG_COMPONENT_TEXT;
            logComponent->value = componentText;

            currPos += componentLen;
        }

        if (nextMatch) {
            int32_t compLen = nextMatch->end - nextMatch->start - 2;
            char* componentName = malloc(compLen + 1);

            strncpy(componentName, source + nextMatch->start + 1, compLen);
            componentName[compLen] = 0;

            Rb_LogComponent* logComponent = &logComponents[numLogComponents++];

            if (strcmp(componentName, COMP_NAME_MESSAGE) == 0) {
                logComponent->type = eRB_LOG_COMPONENT_MESSAGE;
            } else if (strcmp(componentName, COMP_NAME_TAG) == 0) {
                logComponent->type = eRB_LOG_COMPONENT_TAG;
            } else if (strcmp(componentName, COMP_NAME_TIMESTAPM) == 0) {
                logComponent->type = eRB_LOG_COMPONENT_TIMESTAMP;
            } else if (strcmp(componentName, COMP_NAME_LEVEL) == 0) {
                logComponent->type = eRB_LOG_COMPONENT_LEVEL;
            } else if (strcmp(componentName, COMP_NAME_FILE) == 0) {
                logComponent->type = eRB_LOG_COMPONENT_FILE;
            } else if (strcmp(componentName, COMP_NAME_LINE) == 0) {
                logComponent->type = eRB_LOG_COMPONENT_LINE;
            } else if (strcmp(componentName, COMP_NAME_FUNCTION) == 0) {
                logComponent->type = eRB_LOG_COMPONENT_FUNCTION;
            } else if (strcmp(componentName, COMP_NAME_PID) == 0) {
                logComponent->type = eRB_LOG_COMPONENT_PID;
            } else if (strcmp(componentName, COMP_NAME_TID) == 0) {
                logComponent->type = eRB_LOG_COMPONENT_TID;
            } else {
                return RB_ERROR;
            }

            // Advance source parsing
            currPos += compLen + 2;

            // Advance match
            if (currFormatMatch == (int32_t) numFormatMatches - 1) {
                nextMatch = NULL;
            } else {
                nextMatch = &formatMatches[++currFormatMatch];
            }

            free(componentName);
        }
    }

    memset(compiledFormat, 0x00, sizeof(Rb_CompiledFormat));
    compiledFormat->numComponents = numLogComponents;

    compiledFormat->components = (Rb_LogComponent*) malloc(
            sizeof(Rb_LogComponent) * compiledFormat->numComponents);
    memcpy(compiledFormat->components, &logComponents,
            sizeof(Rb_LogComponent) * compiledFormat->numComponents);

    free(formatMatches);
    formatMatches = 0;

    return RB_OK;
}

int32_t Rb_logPriv_parseFormat(const char* source, Rb_FormatMatch** outMatches,
        uint32_t* outNumMatches) {
// List of matches found
    Rb_FormatMatch matches[MAX_FORMAT_COMPONENTS];
    uint32_t numMatches = 0;

    static const char* regexString = "\\{[A-Z]+\\}";

    size_t maxMatches = MAX_FORMAT_COMPONENTS;
    size_t maxGroups = MAX_FORMAT_COMPONENTS;

    regex_t regexCompiled;
    regmatch_t groupArray[maxGroups];
    unsigned int m;
    char * cursor;

    if (regcomp(&regexCompiled, regexString, REG_EXTENDED)) {
        return RB_ERROR;
    };

    m = 0;
    cursor = (char*) source;
    for (m = 0; m < maxMatches; m++) {
        if (regexec(&regexCompiled, cursor, maxGroups, groupArray, 0))
            break;  // No more matches

        uint32_t g = 0;
        uint32_t offset = 0;
        for (g = 0; g < maxGroups; g++) {
            if (groupArray[g].rm_so == (regoff_t) - 1) {
                break;  // No more groups
            }

            if (g == 0) {
                offset = groupArray[g].rm_eo;
            }

            char cursorCopy[strlen(cursor) + 1];
            strcpy(cursorCopy, cursor);
            cursorCopy[groupArray[g].rm_eo] = 0;
            int32_t s = cursor - source;

            matches[numMatches].start = groupArray[g].rm_so + s;
            matches[numMatches].end = groupArray[g].rm_eo + s;
            numMatches++;
        }
        cursor += offset;
    }

    *outNumMatches = numMatches;
    *outMatches = malloc(sizeof(Rb_FormatMatch) * numMatches);
    memcpy(*outMatches, matches, numMatches * sizeof(Rb_FormatMatch));

    regfree(&regexCompiled);

    return RB_OK;
}

char* Rb_logPriv_formatMessage(const Rb_MessageInfo* message,
        const Rb_CompiledFormat* format) {
    // Convert level to string
    char levelStr[2] = "?";
    switch (message->level) {
    case eRB_LOG_VERBOSE:
        levelStr[0] = 'V';
        break;
    case eRB_LOG_DEBUG:
        levelStr[0] = 'D';
        break;
    case eRB_LOG_INFO:
        levelStr[0] = 'I';
        break;
    case eRB_LOG_WARN:
        levelStr[0] = 'W';
        break;
    case eRB_LOG_ERROR:
        levelStr[0] = 'E';
        break;
    case eRB_LOG_FATAL:
        levelStr[0] = 'F';
        break;
    default:
        return NULL;
    }

    // Convert timestamp to string
    char timeStr[RB_STRING_SMALL];
    struct tm* time;
    time = localtime(&message->timestamp);
    if (time == NULL) {
        return NULL;
    }

    if (strftime(timeStr, sizeof(timeStr), "%H:%M:%S", time) == 0) {
        return NULL;
    }

    uint32_t finalMessageSize = 128;
    char* finalMessage = calloc(1, finalMessageSize);

    uint32_t i;
    for (i = 0; i < format->numComponents; i++) {
        switch (format->components[i].type) {
        case eRB_LOG_COMPONENT_TEXT: {
            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, format->components[i].value);
            break;
        }
        case eRB_LOG_COMPONENT_MESSAGE: {
            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, message->message);
            break;
        }
        case eRB_LOG_COMPONENT_LINE: {
            char lineStr[RB_STRING_SMALL];
            sprintf(lineStr, "%llu", (long long unsigned int) message->line);

            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, lineStr);
            break;
        }
        case eRB_LOG_COMPONENT_FILE: {
            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, message->fileName);
            break;
        }
        case eRB_LOG_COMPONENT_FUNCTION: {
            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, message->function);
            break;
        }
        case eRB_LOG_COMPONENT_TIMESTAMP: {
            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, timeStr);
            break;
        }
        case eRB_LOG_COMPONENT_TAG: {
            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, message->tag);
            break;
        }
        case eRB_LOG_COMPONENT_LEVEL: {
            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, levelStr);
            break;
        }
        case eRB_LOG_COMPONENT_TID: {
            char tidStr[RB_STRING_SMALL];
            sprintf(tidStr, "0x%x", message->tid);

            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, tidStr);
            break;
        }
        case eRB_LOG_COMPONENT_PID: {
            char pidStr[RB_STRING_SMALL];
            sprintf(pidStr, "%u", message->pid);

            Rb_Utils_growAppend(&finalMessage, finalMessageSize,
                    &finalMessageSize, pidStr);
            break;
        }
        default:
            free(finalMessage);
            return NULL;
        }
    }

    return finalMessage;
}

