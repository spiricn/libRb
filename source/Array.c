/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Array.h"
#include "rb/Common.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define ARRAY_MAGIC ( 0x11ADBF34 )

#define LOCK_ACQUIRE do{ pthread_mutex_lock(&array->mutex); }while(0)

#define LOCK_RELEASE do{ pthread_mutex_unlock(&array->mutex); }while(0)

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    uint32_t magic;
    FILE* stream;
    char* buffer;
    size_t len;
    pthread_mutex_t mutex;
} ArrayContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static ArrayContext* ArrayPriv_getContext(Rb_ArrayHandle handle);

// Gets rid of the implicit declaration warning
extern FILE *open_memstream(char **bufp, size_t *sizep);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_ArrayHandle Rb_Array_new() {
    ArrayContext* array = (ArrayContext*)calloc(1, sizeof(ArrayContext));

    array->magic = ARRAY_MAGIC;

    array->stream = open_memstream(&array->buffer, &array->len);

    if(array->stream == NULL) {
        free(array);
        return NULL;
    }

    pthread_mutex_init(&array->mutex, NULL);

    return (Rb_ArrayHandle)array;
}

int32_t Rb_Array_free(Rb_ArrayHandle* handle) {
    ArrayContext* array = ArrayPriv_getContext(*handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    fclose(array->stream);
    free(array->buffer);

    pthread_mutex_destroy(&array->mutex);

    free(array);
    *handle = NULL;

    return RB_OK;
}

uint8_t* Rb_Array_data(Rb_ArrayHandle handle) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return NULL;
    }

    LOCK_ACQUIRE;

    fflush(array->stream);

    uint8_t* res = (uint8_t*)array->buffer;

    LOCK_RELEASE;

    return res;
}

uint32_t Rb_Array_size(Rb_ArrayHandle handle) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE;

    fflush(array->stream);

    uint32_t res = array->len;

    LOCK_RELEASE;

    return res;
}

int32_t Rb_Array_tell(Rb_ArrayHandle handle) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE;

    int32_t res = ftell(array->stream);

    LOCK_RELEASE;

    return res;
}

int32_t Rb_Array_seek(Rb_ArrayHandle handle, uint32_t pos) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE;

    int32_t res = fseek(array->stream, pos, SEEK_SET);

    LOCK_RELEASE;

    return res;
}

int32_t Rb_Array_write(Rb_ArrayHandle handle, const void* ptr, uint32_t size) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE;

    int32_t res = fwrite(ptr, 1, size, array->stream);

    LOCK_RELEASE;

    return res;
}

ArrayContext* ArrayPriv_getContext(Rb_ArrayHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    ArrayContext* array = (ArrayContext*)handle;
    if(array->magic != ARRAY_MAGIC) {
        return NULL;
    }

    return array;
}
