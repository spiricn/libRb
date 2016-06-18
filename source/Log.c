/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "Log.h"
#include "Common.h"

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

int32_t RB_log(int32_t level, const char* tag, const char* fmt, ...){
	pthread_mutex_lock(&gGlobalMutex);

    va_list vl;

    int32_t levelFilter = eRB_LOG_VERBOSE;

    if(level < levelFilter) {
		pthread_mutex_unlock(&gGlobalMutex);
		return RB_OK;
    }

    va_start(vl, fmt);
    int32_t size = vsnprintf(NULL, 0, fmt, vl) + 1;
    va_end(vl);

    char* str = (char*) calloc(1, size);

    va_start(vl, fmt);
    vsnprintf(str, size, fmt, vl);
    va_end(vl);

#ifdef ANDROID
    android_printLog(level, tag, str);
#elif __linux__
    char* tagBuffer = (char*)malloc(strlen(str) + strlen(tag) + 2 + 1);

    sprintf(tagBuffer, "[%s]", tag);

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
