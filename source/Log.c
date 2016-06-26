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

#ifdef ANDROID
#include <utils/Log.h>
#endif

/********************************************************/
/*                 Local Module Variables (MODULE)      */
/********************************************************/

pthread_mutex_t gGlobalMutex = PTHREAD_MUTEX_INITIALIZER;

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int32_t RB_log(RB_LogLevel level, const char* tag, const char* fmt, ...){
	pthread_mutex_lock(&gGlobalMutex);

    va_list vl;

    va_start(vl, fmt);
    int32_t size = vsnprintf(NULL, 0, fmt, vl) + 1;
    va_end(vl);

    char* str = (char*) calloc(1, size);

    va_start(vl, fmt);
    vsnprintf(str, size, fmt, vl);
    va_end(vl);

#ifdef ANDROID
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
#elif __linux__
    char* tagBuffer = (char*)malloc(
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
    switch(level) {
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

    fprintf(stdout, "%s\n", tagBuffer);
    fflush(stdout);

    free(tagBuffer);

#else
	#error "Platform not supported"
#endif

    free(str);

    pthread_mutex_unlock(&gGlobalMutex);

	return RB_OK;
}
