#ifndef LIST_H_
#define LIST_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Common.h"

#include <stdint.h>

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Rb_ListHandle;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/**
 * Creates new list.
 *
 *@param[in] elementSize  Size of the individual list element.
 *@return Valid list handle if successful, NULL otherwise.
 */
Rb_ListHandle Rb_List_new(uint32_t elementSize);

/**
 * Frees existing list.
 *
 * @param[in,ou] handle Pointer to a valid list handle.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_List_free(Rb_ListHandle* handle);

/**
 * Adds a new element to the end of the list.
 *
 * @param[in] handle Valid list handle.
 * @param[in] element Pointer to a list element.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_List_add(Rb_ListHandle handle, const void* element);

/**
 * Gets existing list element.
 *
 * @param[in] handle Valid list handle.
 * @param[in] index Element index.
 * @param[out] element Pointer to a element memory.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_List_get(Rb_ListHandle handle, int32_t index, void* element);

/**
 * Removes existing element from the list.
 *
 * @param[in] handle Valid list handle.
 * @param[in] index Element index
 * @return RB_OK on success, negative value otherwise.
 *
 */
int32_t Rb_List_remove(Rb_ListHandle handle, int32_t index);

/**
 * Inserts a new element at the given index.
 *
 * @param[in] handle Valid list handle.
 * @param[in] index Index at which the element will be inserted.
 * @param[in] element Element to be inserted.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_List_insert(Rb_ListHandle handle, int32_t index, const void* element);

/**
 * Gets the number of elements in the list.
 *
 * @param[in] handle Valid list handle.
 * @return Number of elements in the list on success, negative value otherwise.
 */
int32_t Rb_List_getSize(Rb_ListHandle handle);

#ifdef __cplusplus
}
#endif

#endif
