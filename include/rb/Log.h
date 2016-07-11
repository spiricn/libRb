#ifndef RB_LOG_H_
#define RB_LOG_H_

/********************************************************/
/*                 Includes                             */
/********************************************************/

#include <signal.h>
#include <string.h>
#include <stdint.h>

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
#define RB_LOG_BASE_FMT "[%s:%s(%d)]: "

#define RBL(fmt, LEVEL, TAG, ...) \
	do {\
		RB_log(LEVEL, TAG, RB_LOG_BASE_FMT fmt, RB_FILENAME, __FUNCTION__, __LINE__, ## __VA_ARGS__); \
	} while(0)

#define RBLV(fmt, ...) RBL(fmt, eRB_LOG_VERBOSE, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLD(fmt, ...) RBL(fmt, eRB_LOG_DEBUG, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLI(fmt, ...) RBL(fmt, eRB_LOG_INFO, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLW(fmt, ...) RBL(fmt, eRB_LOG_WARN, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLE(fmt, ...) RBL(fmt, eRB_LOG_ERROR, RB_LOG_TAG, ## __VA_ARGS__)
#define RBLF(fmt, ...) RBL(fmt, eRB_LOG_FATAL, RB_LOG_TAG, ## __VA_ARGS__)

#define RB_ASSERT(condition) \
    do{\
        if(!(condition)) \
        { \
            TRACEE("Assertion failure [ %s ]", # condition); \
            raise(SIGSEGV); \
        } \
    }while(0)

/********************************************************/
/*                 Typedefs                             */
/********************************************************/

typedef enum {
	eRB_LOG_INVALID,
	eRB_LOG_VERBOSE,
	eRB_LOG_DEBUG,
	eRB_LOG_INFO,
	eRB_LOG_WARN,
	eRB_LOG_ERROR,
	eRB_LOG_FATAL,
	eRB_LOG_MAX,
 } RB_LogLevel;

/********************************************************/
/*                 Functions Declarations               */
/********************************************************/

int32_t RB_log(RB_LogLevel level, const char* tag, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
