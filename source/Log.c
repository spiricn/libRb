/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Log.h"
#include "rb/Common.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

/********************************************************/
/*                 Defines                              */
/********************************************************/

#ifdef ANDROID
#include <utils/Log.h>
#endif

#define MAX_OUTPUTS ( 16 )

#define LOCK do { pthread_mutex_lock(&gLogContext.mutex); } while(0)
#define UNLOCK do { pthread_mutex_unlock(&gLogContext.mutex); } while(0)

/********************************************************/
/*                 Typedefs                             */
/********************************************************/

typedef struct {
    union {
        struct {
        } stdout;
        struct {

        } logcat;

        struct {
            FILE* fd;
        } file;
    } data;

    Rb_LogOutput type;
    bool enabled;
} LogOutputContext;

typedef struct {
    pthread_mutex_t mutex;
    LogOutputContext outputs[MAX_OUTPUTS];
} LogContext;
/********************************************************/
/*                 Local Module Variables (MODULE)      */
/********************************************************/

static LogContext gLogContext = { PTHREAD_MUTEX_INITIALIZER };

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int32_t Rb_log(RB_LogLevel level, const char* tag, const char* fmt, ...) {
    LOCK
    ;

    va_list vl;

    va_start(vl, fmt);
    int32_t size = vsnprintf(NULL, 0, fmt, vl) + 1;
    va_end(vl);

    char* str = (char*) calloc(1, size);

    va_start(vl, fmt);
    vsnprintf(str, size, fmt, vl);
    va_end(vl);

    int32_t i;
    for (i = 0; i < MAX_OUTPUTS; i++) {
        LogOutputContext* output = &gLogContext.outputs[i];
        if (output->enabled) {
#ifdef ANDROID
            if(output->type == eRB_LOG_OUTPUT_LOGCAT) {
                int32_t androidLevel = ANDROID_LOG_UNKNOWN;

                switch(level) {
                    case eRB_LOG_VERBOSE:
                    androidLevel = ANDROID_LOG_VERBOSE;
                    break;
                    case eRB_LOG_DEBUG:
                    androidLevel = ANDROID_LOG_DEBUG;
                    break;
                    case eRB_LOG_INFO:
                    androidLevel = ANDROID_LOG_INFO;
                    break;
                    case eRB_LOG_WARN:
                    androidLevel = ANDROID_LOG_WARN;
                    break;
                    case eRB_LOG_ERROR:
                    androidLevel = ANDROID_LOG_ERROR;
                    break;
                    case eRB_LOG_FATAL:
                    androidLevel = ANDROID_LOG_FATAL;
                    break;
                    default:
                    androidLevel = ANDROID_LOG_DEFAULT;
                }

                android_printLog(androidLevel, tag, str);
            }
#endif
            if (output->type == eRB_LOG_OUTPUT_STDOUT
                    || output->type == eRB_LOG_OUTPUT_FILE) {
                char* tagBuffer = (char*) malloc(
                // Base message
                        strlen(str) +
                        // Tag
                                strlen(tag) +
                                // Tag brackets
                                2 +
                                // Level letter
                                1 +
                                // Level brackets
                                2 +
                                // Null terminator
                                1);

                char levelStr[2] = "?";
                switch (level) {
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
                }

                sprintf(tagBuffer, "[%s][%s]", levelStr, tag);

                strcat(tagBuffer, str);

                FILE* fd =
                        output->type == eRB_LOG_OUTPUT_STDOUT ?
                                stdout : output->data.file.fd;

                fprintf(fd, "%s\n", tagBuffer);
                fflush(fd);

                free(tagBuffer);
            }
        }
    }

    free(str);

    UNLOCK
    ;

    return RB_OK;
}

int32_t Rb_logAddOutput(Rb_LogOutput type, void* data) {
    LOCK
    ;
    int i;
    LogOutputContext* output = NULL;
    for (i = 0; i < MAX_OUTPUTS; i++) {
        if (!gLogContext.outputs[i].enabled) {
            output = &gLogContext.outputs[i];
            break;
        }
    }

    if (!output) {
        UNLOCK
        ;
        return RB_ERROR;
    }

    output->enabled = true;
    output->type = type;

    if (type == eRB_LOG_OUTPUT_FILE) {
        const char* filePath = data;

        FILE* fd = fopen(filePath, "wb");
        if (fd == NULL) {
            UNLOCK
            ;
            return RB_ERROR;
        }

        output->data.file.fd = fd;
    }

    UNLOCK
    ;

    return RB_OK;
}
