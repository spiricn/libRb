#ifndef ARRAY_H_
#define ARRAY_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "Common.h"
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

ArrayHandle Array_new();

int32_t Array_free(ArrayHandle* handle);

uint8_t* Array_data(ArrayHandle handle);

uint32_t Array_size(ArrayHandle handle);

int32_t Array_tell(ArrayHandle handle);

int32_t Array_seek(ArrayHandle handle, uint32_t pos);

int32_t Array_write(ArrayHandle handle, void* ptr, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
