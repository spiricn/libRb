#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RB_OK ( 0 )
#define RB_TRUE ( 1 )
#define RB_FALSE ( 0 )

#define RB_ERROR_BASE ( -1000 )

#define RB_ERROR ( RB_ERROR_BASE - 1 )
#define RB_DISABLED ( RB_ERROR_BASE - 2 )
#define RB_INVALID_ARG ( RB_ERROR_BASE - 3 )
#define RB_TIMEOUT ( RB_ERROR_BASE - 4 )

#ifdef __cplusplus
}
#endif

#endif
