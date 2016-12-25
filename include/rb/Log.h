#ifndef RB_LOG_H_
#define RB_LOG_H_

/********************************************************/
/*                 Includes                             */
/********************************************************/

#include <rb/Common.h>

#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

/********************************************************/
/*                 Defines                              */
/********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif

#define RB_LOG_TAG "libRB"

#define RB_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define RB_LINE __LINE__
#define RB_FUNCTION __FUNCTION__

#define RBL(fmt, LEVEL, TAG, ...) \
	do {\
		Rb_log(LEVEL, RB_FILENAME, RB_FUNCTION, RB_LINE, TAG, fmt, ## __VA_ARGS__); \
	} while(0)

#define RBLT(fmt, ...) RBL(fmt, eRB_LOG_TRACE, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLV(fmt, ...) RBL(fmt, eRB_LOG_VERBOSE, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLD(fmt, ...) RBL(fmt, eRB_LOG_DEBUG, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLI(fmt, ...) RBL(fmt, eRB_LOG_INFO, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLW(fmt, ...) RBL(fmt, eRB_LOG_WARN, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLE(fmt, ...) RBL(fmt, eRB_LOG_ERROR, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLF(fmt, ...) RBL(fmt, eRB_LOG_FATAL, RB_LOG_TAG, ## __VA_ARGS__)

#define RBL_FN_ENTER do{ RBLT("%s () {", __FUNCTION__); }while(0)
#define RBL_RETURN(x) do{ RBLT("} // %s", __FUNCTION__); return x; }while(0);

#define RB_ASSERT(condition) \
    do{\
        if(!(condition)) \
        { \
            RBLF("Assertion failure [ %s ]", # condition); \
            raise(SIGSEGV); \
        } \
    }while(0)

/********************************************************/
/*                 Typedefs                             */
/********************************************************/

typedef enum {
    eRB_LOG_INVALID,
    eRB_LOG_TRACE,
    eRB_LOG_VERBOSE,
    eRB_LOG_DEBUG,
    eRB_LOG_INFO,
    eRB_LOG_WARN,
    eRB_LOG_ERROR,
    eRB_LOG_FATAL,
    eRB_LOG_MAX,
} RB_LogLevel;

typedef enum {
    eRB_LOG_OUTPUT_STDOUT, eRB_LOG_OUTPUT_FILE,
#ifdef ANDROID
    eRB_LOG_OUTPUT_LOGCAT,
#endif
    eRB_LOG_OUTPUT_CUSTOM, eRB_LOG_OUTPUT_MAX
} Rb_LogOutput;

typedef struct {
    const char* message;
    const char* fileName;
    const char* function;
    uint64_t line;
    time_t timestamp;
    const char* tag;
    RB_LogLevel level;
    uint32_t pid;
    uint32_t tid;
} Rb_MessageInfo;

typedef void (*Rb_LogCustomCallbackFnc)(const Rb_MessageInfo* info,
        const char* finalMessage, void* userData);

typedef struct {
    char format[RB_STRING_SMALL];
    bool enabled;

    union {
        struct {
            FILE* output;
        } file;

        struct {
            Rb_LogCustomCallbackFnc fnc;
            void* userData;
        } custom;
    } data;
} Rb_LogOutputConfig;

/********************************************************/
/*                 Functions Declarations               */
/********************************************************/

int32_t Rb_log(RB_LogLevel level, const char* fileName, const char* function,
        uint64_t line, const char* tag, const char* fmt, ...);

int32_t Rb_log_getOutputConfig(Rb_LogOutput type, Rb_LogOutputConfig* config);

int32_t Rb_log_setOutputConfig(Rb_LogOutput type,
        const Rb_LogOutputConfig* config);

#ifdef __cplusplus
}
#endif

#endif
