/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Common.h"
#include "rb/List.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define LIST_MAGIC ( 0xFF3FA233 )

#define LOCK_ACQUIRE do{ pthread_mutex_lock(&list->mutex); }while(0)

#define LOCK_RELEASE do{ pthread_mutex_unlock(&list->mutex); }while(0)

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct ListNode_t {
    void* element;
    struct ListNode_t* next;
    struct ListNode_t* prev;
} ListNode;

typedef struct {
    uint32_t magic;
    uint32_t size;
    uint32_t elementSize;
    ListNode* head;
    pthread_mutex_t mutex;
} ListContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static ListContext* ListPriv_getContext(ListHandle handle);
static ListNode* ListPriv_getNode(ListContext* list, int32_t index);
static int32_t ListPriv_insertLockless(ListContext* list, int32_t index, const void* element);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

ListHandle List_new(uint32_t elementSize){
    ListContext* list = (ListContext*)calloc(1, sizeof(ListContext));

    list->magic = LIST_MAGIC;
    list->elementSize = elementSize;
    pthread_mutex_init(&list->mutex, NULL);

    return (ListHandle)list;
}

int32_t List_free(ListHandle* handle){
    int32_t rc;

    ListContext* list = ListPriv_getContext(*handle);
    if(list == NULL) {
        return RB_INVALID_ARG;
    }

    while(list->size){
        rc = List_remove(*handle, 0);
        if(rc != RB_OK){
            return rc;
        }
    }

    pthread_mutex_destroy(&list->mutex);

    free(list);
    *handle = NULL;

    return RB_OK;
}

int32_t List_add(ListHandle handle, const void* element){
    ListContext* list = ListPriv_getContext(handle);
    if (list == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE;

    int32_t rc = ListPriv_insertLockless(list, list->size, element);

    LOCK_RELEASE;

    return rc;
}

int32_t List_get(ListHandle handle, int32_t index, void* element){
    ListContext* list = ListPriv_getContext(handle);
    if (list == NULL || element == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE;

    ListNode* node = ListPriv_getNode(list, index);

    if(!node){
        LOCK_RELEASE;
        return RB_INVALID_ARG;
    }

    memcpy(element, node->element, list->elementSize);

    LOCK_RELEASE;

    return RB_OK;
}

int32_t List_remove(ListHandle handle, int32_t index){
    ListContext* list = ListPriv_getContext(handle);
    if (list == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE;

    ListNode* node = ListPriv_getNode(list, index);
    if(!node){
        LOCK_RELEASE;
        return RB_INVALID_ARG;
    }

    if(node->prev){
        node->prev->next = node->next;
    }

    if(node->next){
        node->next->prev = node->prev;
    }

    if(index == 0){
        list->head = node->next;
    }

    list->size--;

    if(!list->size){
        list->head = NULL;
    }

    free(node->element);
    free(node);

    LOCK_RELEASE;

    return RB_OK;
}

int32_t List_insert(ListHandle handle, int32_t index, const void* element){
    ListContext* list = ListPriv_getContext(handle);
    if (list == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE;

    int32_t rc = ListPriv_insertLockless(list, index, element);

    LOCK_RELEASE;

    return rc;
}

int32_t List_getSize(ListHandle handle){
    ListContext* list = ListPriv_getContext(handle);
    if(list == NULL) {
        return RB_INVALID_ARG;
    }

    LOCK_ACQUIRE;

    int32_t res = list->size;

    LOCK_RELEASE;

    return res;
}

ListContext* ListPriv_getContext(ListHandle handle) {
    if(handle == NULL) {
        return NULL;
    }

    ListContext* list = (ListContext*)handle;
    if(list->magic != LIST_MAGIC) {
        return NULL;
    }

    return list;
}

ListNode* ListPriv_getNode(ListContext* list, int32_t index){
    if(index >= (int32_t)list->size){
        return NULL;
    }

    ListNode* node = list->head;

    int32_t currIndex = 0;

    while(currIndex < index){
        if(currIndex == index){
            return node;
        }

        node = node->next;
        currIndex++;
    }

    return node;
}

int32_t ListPriv_insertLockless(ListContext* list, int32_t index, const void* element){
    if((int32_t)list->size > index){
        return RB_INVALID_ARG;
    }

    ListNode* node = (ListNode*)calloc(1, sizeof(ListNode));
    node->element = (void*)calloc(1, list->elementSize);
    memcpy(node->element, element, list->elementSize);
    if(!list->size){
        list->head = node;
    }
    else{
        ListNode* parent = ListPriv_getNode(list, index-1);
        parent->next = node;
        node->prev = parent;
    }

    list->size++;

    return RB_OK;
}
