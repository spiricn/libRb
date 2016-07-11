#ifndef RB_FILESTREAM_H_
#define RB_FILETREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/IOStream.h>

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

int32_t Rb_FileStream_getApi(Rb_IOApi* api);

#ifdef __cplusplus
}
#endif

#endif
