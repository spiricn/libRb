/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/FileStream.h"
#include "rb/Common.h"
#include "rb/Utils.h"
#include "rb/priv/ErrorPriv.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define FILE_STREAM_MAGIC ( 0x56563A54 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    int32_t magic;
    FILE* fd;
    char* uri;
    Rb_IOMode mode;
} FileStreamContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static int32_t FStreamPriv_read(Rb_IOStreamHandle handle, void* data, uint32_t size);

static int32_t FStreamPriv_write(Rb_IOStreamHandle handle, const void* data, uint32_t size);

static int32_t FStreamPriv_tell(Rb_IOStreamHandle handle);

static int32_t FStreamPriv_seek(Rb_IOStreamHandle handle, uint32_t position);

static int32_t FStreamPriv_open(const char* uri, Rb_IOMode mode, Rb_IOStreamHandle* handle);

static int32_t FStreamPriv_close(Rb_IOStreamHandle* handle);

static FileStreamContext* FStreamPriv_getContext(Rb_IOStreamHandle handle);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int32_t Rb_FileStream_getApi(Rb_IOApi* api){
    memset(api, 0x00, sizeof(Rb_IOApi));

    api->close = FStreamPriv_close;
    api->open = FStreamPriv_open;
    api->read = FStreamPriv_read;
    api->write = FStreamPriv_write;
    api->seek= FStreamPriv_seek;
    api->tell= FStreamPriv_tell;

    return RB_OK;
}

int32_t FStreamPriv_read(Rb_IOStreamHandle handle, void* data, uint32_t size){
    FileStreamContext* stream = FStreamPriv_getContext(handle);
    if(stream == NULL){
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return fread(data, 1, size, stream->fd);
}

int32_t FStreamPriv_write(Rb_IOStreamHandle handle, const void* data, uint32_t size){
    FileStreamContext* stream = FStreamPriv_getContext(handle);
    if (stream == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return fwrite(data, 1, size, stream->fd);
}

int32_t FStreamPriv_tell(Rb_IOStreamHandle handle) {
    FileStreamContext* stream = FStreamPriv_getContext(handle);
    if (stream == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return ftell(stream->fd);
}

int32_t FStreamPriv_seek(Rb_IOStreamHandle handle, uint32_t position){
    FileStreamContext* stream = FStreamPriv_getContext(handle);
    if (stream == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    return fseek(stream->fd, position, SEEK_SET);
}

int32_t FStreamPriv_open(const char* uri, Rb_IOMode mode, Rb_IOStreamHandle* handle){
    FileStreamContext* stream = (FileStreamContext*)RB_CALLOC(sizeof(FileStreamContext));

    stream->magic = FILE_STREAM_MAGIC;

    char strMode[8];

    switch(mode){
    case eRB_IO_MODE_READ:
        strcpy(strMode, "rb");
        break;
    case eRB_IO_MODE_WRITE:
        strcpy(strMode, "wb");
        break;
    case eRB_IO_MODE_READ_WRITE:
        strcpy(strMode, "rwb");
        break;
    default:
        RB_FREE(&stream);
        return RB_INVALID_ARG;
    }

    stream->fd = fopen(uri, strMode);
    if(stream->fd == NULL){
        RB_FREE(&stream);
        RB_ERRC(RB_INVALID_ARG, "fopen failed");
    }

    stream->uri = strdup(uri);
    stream->mode = mode;

    *handle = stream;

    return RB_OK;
}

int32_t FStreamPriv_close(Rb_IOStreamHandle* handle){
    FileStreamContext* stream = FStreamPriv_getContext(*handle);
    if (stream == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    if(stream->uri){
        RB_FREE(&stream->uri);
    }

    if(stream->fd){
        fclose(stream->fd);
    }

    RB_FREE(&stream);
    *handle = NULL;

    return RB_OK;
}

FileStreamContext* FStreamPriv_getContext(Rb_IOStreamHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    FileStreamContext* stream = (FileStreamContext*)handle;
    if(stream->magic != FILE_STREAM_MAGIC) {
        return NULL;
    }

    return stream;
}
