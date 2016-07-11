/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/FileStream.h"
#include "rb/Common.h"

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
        return RB_INVALID_ARG;
    }

    return fread(data, 1, size, stream->fd);
}

int32_t FStreamPriv_write(Rb_IOStreamHandle handle, const void* data, uint32_t size){
    FileStreamContext* stream = FStreamPriv_getContext(handle);
    if (stream == NULL) {
        return RB_INVALID_ARG;
    }

    return fwrite(data, 1, size, stream->fd);
}

int32_t FStreamPriv_tell(Rb_IOStreamHandle handle) {
    FileStreamContext* stream = FStreamPriv_getContext(handle);
    if (stream == NULL) {
        return RB_INVALID_ARG;
    }

    return ftell(stream->fd);
}

int32_t FStreamPriv_seek(Rb_IOStreamHandle handle, uint32_t position){
    FileStreamContext* stream = FStreamPriv_getContext(handle);
    if (stream == NULL) {
        return RB_INVALID_ARG;
    }

    return fseek(stream->fd, position, SEEK_SET);
}

int32_t FStreamPriv_open(const char* uri, Rb_IOMode mode, Rb_IOStreamHandle* handle){
    FileStreamContext* stream = (FileStreamContext*)calloc(1, sizeof(FileStreamContext));

    stream->magic = FILE_STREAM_MAGIC;

    char strMode[8];

    switch(mode){
    case eRB_IO_MODE_READ:
        strcpy(strMode, "r");
        break;
    case eRB_IO_MODE_WRITE:
        strcpy(strMode, "w");
        break;
    case eRB_IO_MODE_READ_WRITE:
        strcpy(strMode, "rw");
        break;
    default:
        free(stream);
        return RB_INVALID_ARG;
    }

    stream->fd = fopen(uri, strMode);
    if(stream->fd == NULL){
        free(stream);
        return RB_ERROR;
    }

    stream->uri = strdup(uri);
    stream->mode = mode;

    *handle = stream;

    return RB_OK;
}

int32_t FStreamPriv_close(Rb_IOStreamHandle* handle){
    FileStreamContext* stream = FStreamPriv_getContext(*handle);
    if (stream == NULL) {
        return RB_INVALID_ARG;
    }

    if(stream->uri){
        free(stream->uri);
    }

    if(stream->fd){
        fclose(stream->fd);
    }

    free(stream);
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
