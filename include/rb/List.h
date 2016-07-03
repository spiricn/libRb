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

typedef enum {
    eRB_SORT_ASCEND,
    eRB_SORT_DESCEND
} Rb_SortMode;

typedef void* Rb_ListHandle;

typedef int32_t (*Rb_List_compareFnc)(Rb_ListHandle handle, void* elem1, void* elem2);

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

/**
 * Removes all the elements from the list.
 *
 * @param[in] handle Valid list handle.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_List_clear(Rb_ListHandle handle);

/**
 * Sorts the list elements.
 *
 * @param[in] handle Valid list handle.
 * @param[in] compareFnc Function used to compare two elements of the list. The function should
 *            return -1 if the first elements value is lesser than the value of the second one,
 *            0 if they're the same, and 1 if its greater.
 * @param[in] mode Sorting mode. Determines the order in which the elements will be sorted
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_List_sort(Rb_ListHandle handle, Rb_List_compareFnc compareFnc, Rb_SortMode mode);

/**
 * Swaps the values of two elements in the list.
 *
 * @param[in] handle Valid list handle.
 * @param[in] index1 Index of the first element.
 * @param[in] index2 Index of the second element.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_List_swap(Rb_ListHandle handle, int32_t index1, int32_t index2);

#ifdef __cplusplus
}
#endif

#endif
