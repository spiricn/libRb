/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "Array.h"
#include "Common.h"

#include <stdio.h>
#include <stdlib.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define ARRAY_MAGIC ( 0x11ADBF34 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    uint32_t magic;
    FILE* stream;
    char* buffer;
    size_t len;
} ArrayContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static ArrayContext* ArrayPriv_getContext(ArrayHandle handle);

// Gets rid of the implicit declaration warning
extern FILE *open_memstream(char **bufp, size_t *sizep);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

ArrayHandle Array_new() {
    ArrayContext* array = (ArrayContext*)calloc(1, sizeof(ArrayContext));

    array->magic = ARRAY_MAGIC;

    array->stream = open_memstream(&array->buffer, &array->len);

    if(array->stream == NULL) {
        free(array);
        return NULL;
    }

    return (ArrayHandle)array;
}

int32_t Array_free(ArrayHandle* handle) {
    ArrayContext* array = ArrayPriv_getContext(*handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    fclose(array->stream);
    free(array->buffer);

    free(array);
    *handle = NULL;

    return RB_OK;
}

uint8_t* Array_data(ArrayHandle handle) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return NULL;
    }

    fflush(array->stream);

    return (uint8_t*)array->buffer;
}

uint32_t Array_size(ArrayHandle handle) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    fflush(array->stream);

    return array->len;
}

int32_t Array_tell(ArrayHandle handle) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    return ftell(array->stream);
}

int32_t Array_seek(ArrayHandle handle, uint32_t pos) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    return fseek(array->stream, pos, SEEK_SET);
}

int32_t Array_write(ArrayHandle handle, void* ptr, uint32_t size) {
    ArrayContext* array = ArrayPriv_getContext(handle);
    if(array == NULL) {
        return RB_INVALID_ARG;
    }

    return fwrite(ptr, 1, size, array->stream);
}

ArrayContext* ArrayPriv_getContext(ArrayHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    ArrayContext* array = (ArrayContext*)handle;
    if(array->magic != ARRAY_MAGIC) {
        return NULL;
    }

    return array;
}
