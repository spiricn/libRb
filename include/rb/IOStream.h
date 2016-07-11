#ifndef RB_IOSTREAM_H_
#define RB_IOSTREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <limits.h>
#include <stdint.h>


/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef enum {
    eRB_IO_MODE_READ,
    eRB_IO_MODE_WRITE,
    eRB_IO_MODE_READ_WRITE,
} Rb_IOMode;

typedef void* Rb_IOStreamHandle;

typedef int32_t (*Rb_IOReadFnc)(Rb_IOStreamHandle handle, void* data, uint32_t size);
typedef int32_t (*Rb_IOWriteFnc)(Rb_IOStreamHandle handle, const void* data, uint32_t size);
typedef int32_t (*Rb_IOTellFnc)(Rb_IOStreamHandle handle);
typedef int32_t (*Rb_IOSeekFnc)(Rb_IOStreamHandle handle, uint32_t position);
typedef int32_t (*Rb_IOOpenFnc)(const char* uri, Rb_IOMode mode, Rb_IOStreamHandle* handle);
typedef int32_t (*Rb_IOCloseFnc)(Rb_IOStreamHandle* handle);

typedef struct {
    Rb_IOReadFnc read;
    Rb_IOWriteFnc write;
    Rb_IOOpenFnc open;
    Rb_IOCloseFnc close;
    Rb_IOTellFnc tell;
    Rb_IOSeekFnc seek;
} Rb_IOApi;

typedef struct {
    Rb_IOApi api;
    Rb_IOStreamHandle handle;
} Rb_IOStream;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/**
 * Writes a formatted string to a stream.
 *
 * @param[in] stream Output stream.
 * @param[in] fmt String format.
 *
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_IOStream_print(const Rb_IOStream* stream, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
