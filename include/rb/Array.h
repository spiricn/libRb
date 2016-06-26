#ifndef ARRAY_H_
#define ARRAY_H_

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

typedef void* ArrayHandle;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/**
 * Creates new array.
 *
 * @return Array handle on success, negative value otherwise.
 */
ArrayHandle Array_new();

/**
 * Frees existing array.
 *
 * @param[in,out] handle Pointer to a valid array handle.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Array_free(ArrayHandle* handle);

/**
 * Acquires a pointer to the arrays data.
 *
 * @param[in] handle Valid array handle.
 * @return Pointer to the arrays data on success, NULL otherwise.
 */
uint8_t* Array_data(ArrayHandle handle);

/**
 * Acquires array data size.
 *
 * @param[in] handle Valid array handle.
 * @return Array data size on success, negative value otherwise.
 */
uint32_t Array_size(ArrayHandle handle);

/**
 * Acquires arrays current write position.
 *
 * @param[in] handle Valid array handle.
 * @return Arrays write position on success, negative value otherwise.
 */
int32_t Array_tell(ArrayHandle handle);

/**
 * Seeks to a new write position.
 *
 * @param[in] handle Valid array handle.
 * @param[in] pos Write position in bytes.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Array_seek(ArrayHandle handle, uint32_t pos);

/**
 * Writes new data to the arrays write position.
 * @param[in] handle Valid array handle.
 * @param[in] ptr Data pointer.
 * @param[in] size Data size.
 * @return RB_OK on success, negative value otherwijse.
 */
int32_t Array_write(ArrayHandle handle, const void* ptr, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
