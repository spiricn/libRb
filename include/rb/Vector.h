#ifndef RB_VECTOR_H_
#define RB_VECTOR_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/Common.h>

#include <stdint.h>

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Rb_VectorHandle;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/**
 * Creates a new vector.
 *
 *@param[in] elementSize  Size of the individual vector element.
 *@return Valid vector handle if successful, NULL otherwise.
 */
Rb_VectorHandle Rb_Vector_new(uint32_t elementSize);

/**
 * Frees existing vector.
 *
 * @param[in,ou] handle Pointer to a valid vector handle.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Vector_free(Rb_VectorHandle* handle);

/**
 * Adds a new element to the end of the vector.
 *
 * @param[in] handle Valid vector handle.
 * @param[in] element Pointer to a vector element.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Vector_add(Rb_VectorHandle handle, const void* element);

/**
 * Adds multiple elements at the end of the vector
 *
 * @param[in] handle Valid vector handle.
 * @param[in] element Pointer to a vector element array.
 * @param[in] numElements Number of elements in the array.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Vector_addRange(Rb_VectorHandle handle, const void* elements, int32_t numElements);

/**
 * Remove single element from the vector. May not change the size of the underlying buffer.
 *
 * @param[in] handle Valid vector handle.
 * @param[in] index Index of the element to remove.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Vector_remove(Rb_VectorHandle handle, int32_t index);

/**
 * Remove multiple elements from the vector. May not change the size of the underlying buffer.
 *
 * @param[in] handle Valid vector handle.
 * @param[in] index Index of the element to remove.
 * @param[in] numElements Number of elements to remove starting with index.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Vector_removeRange(Rb_VectorHandle handle, int32_t startIndex, int32_t numElements);


/**
 * Gets the number of elements stored in the vector.
 *
 * @param[in] handle Valid vector handle.
 * @return Number of elements in the vector on success, negative value otherwise.
 */
int32_t Rb_Vector_getNumElements(Rb_VectorHandle handle);

/**
 * Gets the size of the underlying vector buffer. Must be greater or equal to the number of elements * element size.
 *
 * @param[in] handle Valid vector handle.
 * @return Number of elements in the vector on success, negative value otherwise.
 */
int32_t Rb_Vector_getSize(Rb_VectorHandle handle);

/**
 * Gets a pointer to a single element of the vector. This pointer might become invalid after other vector calls.
 *
 * @param[in] handle Valid vector handle.
 * @return Pointer to the vector element if successful, NULLL otherwise.
 */
void* Rb_Vector_get(Rb_VectorHandle handle, int32_t index);

/**
 * Gets a pointer to the underlying vector buffer.
 *
 * @param[in] handle Valid vector handle.
 * @return Pointer to the vector buffer if successful, NULLL otherwise.
 */
void* Rb_Vector_getData(Rb_VectorHandle handle);

/**
 * Removes all the elements from the vector. May not actually free memory.
 *
 * @param[in] handle Valid vector handle.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Vector_clear(Rb_VectorHandle handle);

#ifdef __cplusplus
}
#endif

#endif
