/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Log.h"
#include "rb/Common.h"
#include "rb/priv/LogPriv.h"
#include "rb/Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <regex.h>
#include <sys/types.h>
#include <unistd.h>

/********************************************************/
/*                 Defines                              */
/********************************************************/

#ifdef ANDROID
#include <android/log.h>
#endif

#define LOCK do { pthread_mutex_lock(&gMutex); } while(0)
#define UNLOCK do { pthread_mutex_unlock(&gMutex); } while(0)

#define DEFAULT_FORMAT "[{TIMESTAMP}][{LEVEL}/{FILE}:{FUNCTION}({LINE})]: {MESSAGE}"

#ifdef ANDROID
#define DEFAULT_FORMAT_LOGCAT "[{FILE}:{FUNCTION}({LINE})]: {MESSAGE}"
#endif

/********************************************************/
/*                 Typedefs                             */
/********************************************************/

typedef struct {
    Rb_LogOutput type;
    Rb_LogOutputConfig config;
    Rb_CompiledFormat compiledFormat;
} LogOutputContext;

typedef struct {

    bool initialized;
    LogOutputContext outputs[eRB_LOG_OUTPUT_MAX];
} LogContext;

/********************************************************/
/*                 Local Module Variables (MODULE)      */
/********************************************************/

static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;
static bool gContextInitialized = false;
static LogContext gLogContext;

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

static void Rb_logPriv_initialize();
#ifdef ANDROID
static int32_t Rb_logPriv_outputLogcat(const Rb_MessageInfo* info, const char* finalMessage);
#endif

static int32_t Rb_logPriv_outputFile(const Rb_MessageInfo* info,
        const char* finalMessage, FILE* file);

static int32_t Rb_logPriv_logMessage(const Rb_MessageInfo* message,
        const LogOutputContext* output);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

void Rb_logPriv_initialize() {
    memset(&gLogContext, 0x00, sizeof(LogContext));

    int i;
    for (i = 0; i < eRB_LOG_OUTPUT_MAX; i++) {
        LogOutputContext* output = &gLogContext.outputs[i];

        memset(output, 0x00, sizeof(LogOutputContext));

        output->type = i;
        output->config.enabled = false;
        output->config.logLevel = eRB_LOG_ALL;
        strcpy(output->config.format, DEFAULT_FORMAT);

#if ANDROID
        if(output->type == eRB_LOG_OUTPUT_LOGCAT) {
            strcpy(output->config.format, DEFAULT_FORMAT_LOGCAT);
        }
#endif

        Rb_logPriv_compileFormat(output->config.format,
                &output->compiledFormat);

    }

    // Enable logcat by default on Android, otherwise stdout
#if ANDROID
    gLogContext.outputs[eRB_LOG_OUTPUT_LOGCAT].config.enabled = true;
#else
    gLogContext.outputs[eRB_LOG_OUTPUT_STDOUT].config.enabled = true;
#endif

    gContextInitialized = true;
}

int32_t Rb_logPriv_logMessage(const Rb_MessageInfo* message,
        const LogOutputContext* output) {
    if(message->level < output->config.logLevel){
        return RB_OK;
    }

    int rc;
    char* finalMessage = NULL;

    rc = RB_OK;

    finalMessage = Rb_logPriv_formatMessage(message, &output->compiledFormat);

    switch (output->type) {
#ifdef ANDROID
    case eRB_LOG_OUTPUT_LOGCAT: {
        rc = Rb_logPriv_outputLogcat(message, finalMessage);
        if(rc != RB_OK) {
            goto finish;
        }

        break;
    }

#endif
    case eRB_LOG_OUTPUT_STDOUT:
    case eRB_LOG_OUTPUT_FILE: {
        FILE* fd =
                output->type == eRB_LOG_OUTPUT_STDOUT ?
                        stdout : output->config.data.file.output;
        if (fd) {
            rc = Rb_logPriv_outputFile(message, finalMessage, fd);
            if (rc != RB_OK) {
                goto finish;
            }
        }
        break;
    }

    case eRB_LOG_OUTPUT_CUSTOM: {
        if (output->config.data.custom.fnc) {
            output->config.data.custom.fnc(message, finalMessage,
                    output->config.data.custom.userData);
        }
        break;
    }
    default:
        rc = RB_ERROR;
        goto finish;
    }

    RB_FREE(&finalMessage);
    finalMessage = NULL;

    finish:

    if (finalMessage) {
        RB_FREE(&finalMessage);
        finalMessage = NULL;
    }

    return rc;
}

int32_t Rb_log(RB_LogLevel level, const char* fileName, const char* function,
        uint64_t line, const char* tag, const char* fmt, ...) {
    LOCK
    ;

    if (!gContextInitialized) {
        Rb_logPriv_initialize();
    }

    char* finalMessage = NULL;
    int32_t rc = RB_OK;

    // Format message
    va_list vl;
    va_start(vl, fmt);
    char* message = Rb_Utils_printv(fmt, vl);
    va_end(vl);

    Rb_MessageInfo messageInfo;
    memset(&messageInfo, 0x00, sizeof(Rb_MessageInfo));
    messageInfo.fileName = fileName;
    messageInfo.function = function;
    messageInfo.line = line;
    messageInfo.timestamp = time(NULL);
    messageInfo.tag = tag;
    messageInfo.level = level;
    messageInfo.pid = getpid();
    messageInfo.tid = pthread_self();

    char* pch = strtok(message, "\n");
    while (pch != NULL) {
        if (!strlen(pch)) {
            continue;
        }

        messageInfo.message = pch;

        int i;
        for (i = 0; i < eRB_LOG_OUTPUT_MAX; i++) {
            LogOutputContext* output = &gLogContext.outputs[i];
            if (!output->config.enabled) {
                continue;
            }

            rc = Rb_logPriv_logMessage(&messageInfo, output);
            if (rc != RB_OK) {
                break;
            }
        }

        pch = strtok(NULL, "\n");
    }

    if (message) {
        RB_FREE(&message);
        message = NULL;
    }

    UNLOCK
    ;

    return rc;
}

int32_t Rb_log_getOutputConfig(Rb_LogOutput type, Rb_LogOutputConfig* config) {
    LOCK
    ;

    if (!gContextInitialized) {
        Rb_logPriv_initialize();
    }

    memcpy(config, &gLogContext.outputs[type].config,
            sizeof(Rb_LogOutputConfig));

    UNLOCK
    ;

    return 0;
}

int32_t Rb_log_setOutputConfig(Rb_LogOutput type,
        const Rb_LogOutputConfig* config) {
    int rc;
    LOCK
    ;

    if (!gContextInitialized) {
        Rb_logPriv_initialize();
    }

    LogOutputContext* ctx = &gLogContext.outputs[type];

    // Compile format
    if (ctx->compiledFormat.components) {
        RB_FREE(&ctx->compiledFormat.components);
        ctx->compiledFormat.components = NULL;
        ctx->compiledFormat.numComponents = 0;
    }

    rc = Rb_logPriv_compileFormat(config->format, &ctx->compiledFormat);
    if (rc != RB_OK) {
        UNLOCK
        ;
        return rc;
    }

    memcpy(&ctx->config, config, sizeof(Rb_LogOutputConfig));

    UNLOCK
    ;

    return RB_OK;
}

#ifdef ANDROID
int32_t Rb_logPriv_outputLogcat(const Rb_MessageInfo* info,
        const char* finalMessage) {
    int32_t androidLevel = ANDROID_LOG_UNKNOWN;

    switch (info->level) {
        case eRB_LOG_VERBOSE:
        case eRB_LOG_TRACE:
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

    __android_log_write(androidLevel, info->tag, finalMessage);

    return RB_OK;
}
#endif

int32_t Rb_logPriv_outputFile(const Rb_MessageInfo* info,
        const char* finalMessage, FILE* fd) {
    RB_UNUSED(info);

    int32_t messageSize = strlen(finalMessage) + 1;

    if (fprintf(fd, "%s\n", finalMessage) != messageSize) {
        return RB_ERROR;
    }
    if (fflush(fd) != 0) {
        return RB_ERROR;

    }
    return RB_OK;
}

int32_t Rb_log_terminate(){
    if(!gContextInitialized){
        return RB_OK;
    }

    int32_t i;
    int32_t j;

    for (i = 0; i < eRB_LOG_OUTPUT_MAX; i++) {
        LogOutputContext* output = &gLogContext.outputs[i];

        for(j =0; j<(int32_t)output->compiledFormat.numComponents; j++){
            RB_FREE(&output->compiledFormat.components[j].value);
        }

        RB_FREE(&output->compiledFormat.components);
        output->compiledFormat.numComponents = 0;
    }

    gContextInitialized = false;

    return RB_OK;
}
