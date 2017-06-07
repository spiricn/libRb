/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Common.h"
#include "rb/Vector.h"
#include "rb/Utils.h"
#include "rb/priv/ErrorPriv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define VECTOR_MAGIC ( 0xAAF21111 )

#define LOCK_ACQUIRE do{ pthread_mutex_lock(&vec->mutex); }while(0)

#define LOCK_RELEASE do{ pthread_mutex_unlock(&vec->mutex); }while(0)

#define SIZE_INCREASE_STEP ( 16 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    uint32_t magic;
    uint32_t size;
    uint32_t elementSize;
    uint32_t numElements;
    void* data;
    pthread_mutex_t mutex;
} VectorContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static VectorContext* VectorPriv_getContext(Rb_VectorHandle handle);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_VectorHandle Rb_Vector_new(uint32_t elementSize) {
    if (elementSize == 0) {
        RB_ERR("Invalid element size");
        return NULL;
    }

    VectorContext* vec = (VectorContext*) RB_CALLOC(sizeof(VectorContext));

    vec->magic = VECTOR_MAGIC;
    vec->elementSize = elementSize;
    pthread_mutex_init(&vec->mutex, NULL);

    return (Rb_VectorHandle) vec;
}

int32_t Rb_Vector_free(Rb_VectorHandle* handle) {
    int32_t rc;

    VectorContext* vec = VectorPriv_getContext(*handle);
    if (vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    if (vec->data) {
        RB_FREE(&vec->data);
        vec->size = 0;
        vec->numElements = 0;
    }

    pthread_mutex_destroy(&vec->mutex);

    RB_FREE(&vec);
    *handle = NULL;

    return RB_OK;
}

int32_t Rb_Vector_add(Rb_VectorHandle handle, const void* element) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if (vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    LOCK_ACQUIRE
    ;

    if ((vec->numElements + 1) * vec->elementSize > vec->size) {
        // Resize
        vec->size += vec->elementSize * SIZE_INCREASE_STEP;

        if (vec->data) {
            vec->data = RB_REALLOC(vec->data, vec->size);
        } else {
            vec->data = RB_MALLOC(vec->size);
        }
    }

    memcpy(vec->data + (vec->numElements * vec->elementSize), element,
            vec->elementSize);
    vec->numElements++;

    LOCK_RELEASE
    ;

    return RB_OK;
}

int32_t Rb_Vector_getNumElements(Rb_VectorHandle handle) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if (vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    LOCK_ACQUIRE
    ;

    int32_t res = vec->numElements;

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_Vector_getSize(Rb_VectorHandle handle) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if (vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    LOCK_ACQUIRE
    ;

    int32_t res = vec->size;

    LOCK_RELEASE
    ;

    return res;
}

void* Rb_Vector_get(Rb_VectorHandle handle, int32_t index) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if (vec == NULL) {
        RB_ERR("Invalid handle");
        return NULL;
    }

    LOCK_ACQUIRE
    ;

    void* res = NULL;

    if (index >= vec->numElements) {
        RB_ERR("Element index out of bounds: %d >= %d", index,
                vec->numElements);
    } else {
        res = vec->data + (index * vec->elementSize);
    }

    LOCK_RELEASE
    ;

    return res;
}

void* Rb_Vector_getData(Rb_VectorHandle handle) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if (vec == NULL) {
        RB_ERR("Invalid handle");
        return NULL;
    }

    LOCK_ACQUIRE
    ;

    void* res = vec->data;

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_Vector_clear(Rb_VectorHandle handle) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if (vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    LOCK_ACQUIRE
    ;

    if (vec->data) {
        RB_FREE(&vec->data);
        vec->numElements = 0;
        vec->size = 0;
    }

    LOCK_RELEASE
    ;

    return RB_OK;
}

VectorContext* VectorPriv_getContext(Rb_VectorHandle handle) {
    if (handle == NULL) {
        return NULL;
    }

    VectorContext* vec = (VectorContext*) handle;
    if (vec->magic != VECTOR_MAGIC) {
        return NULL;
    }

    return vec;
}
