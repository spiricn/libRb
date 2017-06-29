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

static int32_t Rb_VectorPriv_addRange(VectorContext* vec, int32_t startIndex,
        const void* elements, int32_t numElements);

static int32_t Rb_VectorPriv_removeRange(VectorContext* vec, int32_t startIndex,
        int32_t numElements);

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

int32_t Rb_Vector_insert(Rb_VectorHandle handle, int32_t index, const void* element){
    VectorContext* vec = VectorPriv_getContext(handle);
    if(vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    } else if(index < 0 || element == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid argument");
    }

    LOCK_ACQUIRE
    ;

    int32_t res = Rb_VectorPriv_addRange(vec, index, element, 1);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_Vector_insertRange(Rb_VectorHandle handle, int32_t startIndex, const void* elements, int32_t numElements){
    VectorContext* vec = VectorPriv_getContext(handle);
    if(vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    } else if(startIndex < 0 || elements == NULL || numElements <= 0) {
        RB_ERRC(RB_INVALID_ARG, "Invalid argument");
    }
    LOCK_ACQUIRE
    ;

    int32_t res = Rb_VectorPriv_addRange(vec, startIndex, elements,
            numElements);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_Vector_remove(Rb_VectorHandle handle, int32_t index) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if(vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    } else if(index < 0) {
        RB_ERRC(RB_INVALID_ARG, "Invalid argument");
    }

    LOCK_ACQUIRE
    ;

    int32_t res = Rb_VectorPriv_removeRange(vec, index, 1);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_Vector_removeRange(Rb_VectorHandle handle, int32_t startIndex,
        int32_t numElements) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if(vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    } else if(startIndex < 0 || numElements <= 0) {
        RB_ERRC(RB_INVALID_ARG, "Invalid argument");
    }

    LOCK_ACQUIRE
    ;

    int32_t res = Rb_VectorPriv_removeRange(vec, startIndex, numElements);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_Vector_addRange(Rb_VectorHandle handle, const void* elements,
        int32_t numElements) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if (vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    if (elements == NULL || numElements <= 0) {
        RB_ERRC(RB_INVALID_ARG, "Invalid argument");
    }

    LOCK_ACQUIRE
    ;

    int32_t res = Rb_VectorPriv_addRange(vec, vec->numElements, elements, numElements);

    LOCK_RELEASE
    ;

    return res;
}

int32_t Rb_VectorPriv_removeRange(VectorContext* vec, int32_t startIndex,
        int32_t numElements) {
    int32_t endOffset = (startIndex + numElements) * vec->elementSize;
    void* startAddress = vec->data + (startIndex * vec->elementSize);
    void* endAddress = vec->data + endOffset;

    int32_t remainderSize = vec->size - endOffset;
    memmove(startAddress, endAddress, remainderSize);

    vec->numElements -= numElements;

    return RB_OK;
}

int32_t Rb_VectorPriv_addRange(VectorContext* vec, int32_t startIndex, const void* elements,
        int32_t numElements) {
    if ((vec->numElements + numElements) * vec->elementSize > vec->size) {
        // TODO Resize exponentially

        // Resize
        vec->size += vec->elementSize * numElements;

        if (vec->data) {
            vec->data = RB_REALLOC(vec->data, vec->size);
        } else {
            vec->data = RB_MALLOC(vec->size);
        }
    }

    if(startIndex == vec->numElements) {
        // Push back
        memcpy(vec->data + (vec->numElements * vec->elementSize), elements,
                vec->elementSize * numElements);
    } else {
        // Inserting, so first make room for new elements
        int32_t blockSize = vec->size - (numElements * vec->elementSize);

        void* oldAddr = vec->data + (startIndex * vec->elementSize);
        void* newAddr = vec->data
                + ((startIndex + numElements) * vec->elementSize);
        memmove(oldAddr, newAddr, numElements * vec->elementSize);
        ;

        // Copy new data
        memcpy(oldAddr, elements, numElements * vec->elementSize);
    }

    vec->numElements += numElements;

    return RB_OK;
}

int32_t Rb_Vector_add(Rb_VectorHandle handle, const void* element) {
    VectorContext* vec = VectorPriv_getContext(handle);
    if (vec == NULL) {
        RB_ERRC(RB_INVALID_ARG, "Invalid handle");
    }

    LOCK_ACQUIRE
    ;

    int32_t rc = Rb_VectorPriv_addRange(vec, vec->numElements, element, 1);

    LOCK_RELEASE
    ;

    return rc;
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
