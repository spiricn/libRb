#ifndef RB_PREFS_PRIV_H_
#define RB_PREFS_PRIV_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Prefs.h"

/*******************************************************/
/*              Defines                                */
/*******************************************************/


/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Rb_VariantType type;

    union {
        int32_t int32Val;
        int64_t int64Val;
        float floatVal;
        char* stringVal;

        struct {
            void* data;
            uint32_t size;
        } blobVal;
    } val;
} Variant;

typedef struct {
    char* key;
    Variant value;
} PrefEntry;

typedef struct {
    uint32_t magic;
    Rb_ListHandle entries;
    Rb_PrefsBackend backend;
} PrefsContext;

#ifdef __cplusplus
}
#endif

#endif
