/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/priv/PrefsBackend.h"
#include "rb/Utils.h"
#include "rb/priv/ErrorPriv.h"

#include <string.h>
#include <stdlib.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define PREFS_BACKEND_MAGIC ( 0x6369AABC )

#define SYNTAX_VERSION_MAJOR ( 1 )
#define SYNTAX_VERSION_MINOR ( 0 )
#define SYNTAX_VERSION_PATCH ( 0 )

#define SYNTAX_VERSION RB_VERSION_NUMBER(SYNTAX_VERSION_MAJOR, SYNTAX_VERSION_MINOR, SYNTAX_VERSION_PATCH)

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    int32_t magic;
    int64_t syntaxVersion;
    int32_t numEntries;
} PrefsBackend_Header;


/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static int32_t PrefsBackendPriv_writeVar(Rb_PrefsHandle handle, const Rb_IOStream* stream, const char* key);

static int32_t PrefsBackendPriv_readVar(Rb_PrefsHandle handle, const Rb_IOStream* stream);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int32_t Rb_PrefsBackendSave(Rb_PrefsHandle handle, const Rb_IOStream* stream){
    PrefsBackend_Header header;
    memset(&header, 0x00, sizeof(PrefsBackend_Header));

    header.magic = PREFS_BACKEND_MAGIC;
    header.syntaxVersion = SYNTAX_VERSION;
    header.numEntries = Rb_Prefs_getNumEntries(handle);

    if (stream->api.write(stream->handle, &header, sizeof(PrefsBackend_Header))
            != sizeof(PrefsBackend_Header)) {
        return RB_ERROR;
    }

    int32_t i;
    int32_t rc;

    for (i = 0; i < Rb_Prefs_getNumEntries(handle); i++){
        const char* key = NULL;

        rc = Rb_Prefs_getKey(handle, i, &key);
        if(rc != RB_OK){
            return rc;
        }

        rc = PrefsBackendPriv_writeVar(handle, stream, key);
        if(rc != RB_OK){
            RB_ERRC(rc, "Error writing value");
        }
    }

    return RB_OK;
}

int32_t Rb_PrefsBackendLoad(Rb_PrefsHandle handle, const Rb_IOStream* stream){
    PrefsBackend_Header header;
    if(stream->api.read(stream->handle, &header, sizeof(PrefsBackend_Header))!= sizeof(PrefsBackend_Header)){
        return RB_ERROR;
    }

    if(header.magic != PREFS_BACKEND_MAGIC){
        RB_ERRC(RB_ERROR, "Invalid data header");
    }

    if(header.syntaxVersion != SYNTAX_VERSION){
        RB_ERRC(RB_ERROR, "Invalid syntax version");
    }

    int32_t i;
    int32_t rc;

    rc = Rb_Prefs_clear(handle);
    if(rc != RB_OK){
        return rc;
    }

    for(i=0; i<header.numEntries; i++){
        rc = PrefsBackendPriv_readVar(handle, stream);
        if(rc != RB_OK){
            RB_ERRC(rc, "Error reading value");
        }
    }

    return RB_OK;
}


int32_t PrefsBackendPriv_writeVar(Rb_PrefsHandle handle, const Rb_IOStream* stream, const char* key){
    int32_t rc;

    Rb_VariantType type;
    rc = Rb_Prefs_getEntryType(handle, key, &type);
    if(rc != RB_OK){
        return rc;
    }

    // Key
    const int32_t keySize = strlen(key) + 1;
    if(stream->api.write(stream->handle, &keySize, sizeof(int32_t)) != sizeof(int32_t)){
        return RB_ERROR;
    }

    rc = Rb_IOStream_print(stream, "%s", key);
    if(rc != RB_OK){
        return rc;
    }

    // Type
    if(stream->api.write(stream->handle, &type, sizeof(int32_t)) != sizeof(int32_t)){
        return RB_ERROR;
    }

    switch(type){
    case eRB_VAR_TYPE_INT32: {
        int32_t val;

        rc = Rb_Prefs_getInt32(handle, key, &val);
        if(rc != RB_OK){
            return rc;
        }

        if(stream->api.write(stream->handle, &val, sizeof(int32_t)) != sizeof(int32_t)){
            return RB_ERROR;
        }
        break;
    }
    case eRB_VAR_TYPE_INT64: {
        int64_t val;

        rc = Rb_Prefs_getInt64(handle, key, &val);
        if(rc != RB_OK){
            return rc;
        }

        if(stream->api.write(stream->handle, &val, sizeof(int64_t)) != sizeof(int64_t)){
            return RB_ERROR;
        }
        break;
    }
    case eRB_VAR_TYPE_FLOAT: {
        float val;

        rc = Rb_Prefs_getFloat(handle, key, &val);
        if(rc != RB_OK){
            return rc;
        }

        if(stream->api.write(stream->handle, &val, sizeof(float)) != sizeof(float)){
            return RB_ERROR;
        }
        break;
    }
    case eRB_VAR_TYPE_STRING: {
        char* val;

        rc = Rb_Prefs_getString(handle, key, &val);
        if(rc != RB_OK){
            return rc;
        }

        const int32_t strSize = strlen(val) + 1;

        if(stream->api.write(stream->handle, &strSize, sizeof(int32_t)) != sizeof(int32_t)){
            return RB_ERROR;
        }

        rc = Rb_IOStream_print(stream, "%s", val);
        if(rc != RB_OK){
            return rc;
        }

        RB_FREE(&val);

        break;
    }
    case eRB_VAR_TYPE_BLOB: {
        void* val;
        uint32_t size;

        rc = Rb_Prefs_getBlob(handle, key, &val, &size);
        if(rc != RB_OK){
            return rc;
        }

        if(stream->api.write(stream->handle, &size, sizeof(int32_t)) != sizeof(int32_t)){
            return RB_ERROR;
        }

        if(stream->api.write(stream->handle, val, size) != (int32_t)size){
            return RB_ERROR;
        }

        RB_FREE(&val);
        break;
    }
    default:
        return RB_INVALID_ARG;
    }

    return RB_OK;
}

int32_t PrefsBackendPriv_readVar(Rb_PrefsHandle handle, const Rb_IOStream* stream){
    int32_t rc;

    int32_t size;
    Rb_VariantType type;

    // Key
    if(stream->api.read(stream->handle, &size, sizeof(int32_t)) != sizeof(int32_t)){
        return RB_ERROR;
    }

    char* key = (char*)RB_CALLOC(size);
    if(stream->api.read(stream->handle, key, size) != size){
        return RB_ERROR;
    }

    // Type
    if(stream->api.read(stream->handle, &type, sizeof(int32_t)) != sizeof(int32_t)){
        return RB_ERROR;
    }

    switch(type){
    case eRB_VAR_TYPE_INT32: {
        int32_t val;
        if(stream->api.read(stream->handle, &val, sizeof(int32_t)) != sizeof(int32_t)){
            return RB_ERROR;
        }

        rc = Rb_Prefs_putInt32(handle, key, val);
        if(rc != RB_OK){
            return rc;
        }

        break;
    }
    case eRB_VAR_TYPE_INT64: {
        int64_t val;
        if(stream->api.read(stream->handle, &val, sizeof(int64_t)) != sizeof(int64_t)){
            return RB_ERROR;
        }

        rc = Rb_Prefs_putInt64(handle, key, val);
        if(rc != RB_OK){
            return rc;
        }

        break;
    }
    case eRB_VAR_TYPE_FLOAT: {
        float val;
        if(stream->api.read(stream->handle, &val, sizeof(float)) != sizeof(float)){
            return RB_ERROR;
        }

        rc = Rb_Prefs_putFloat(handle, key, val);
        if(rc != RB_OK){
            return rc;
        }
        break;
    }
    case eRB_VAR_TYPE_STRING: {
        if(stream->api.read(stream->handle, &size, sizeof(int32_t)) != sizeof(int32_t)){
            return RB_ERROR;
        }

        char* val = (char*)RB_CALLOC(size);
        if(stream->api.read(stream->handle, val, size) != size){
            RB_FREE(&val);
            return RB_ERROR;
        }

        rc = Rb_Prefs_putString(handle, key, val);
        if(rc != RB_OK){
            RB_FREE(&val);
            return rc;
        }

        RB_FREE(&val);
        break;
    }
    case eRB_VAR_TYPE_BLOB: {
        if(stream->api.read(stream->handle, &size, sizeof(int32_t)) != sizeof(int32_t)){
            return RB_ERROR;
        }

        void* data = RB_CALLOC(size);

        if(stream->api.read(stream->handle, data, size) != size){
            RB_FREE(&data);
            return RB_ERROR;
        }

        rc = Rb_Prefs_putBlob(handle, key, data, size);
        if(rc != RB_OK){
            RB_FREE(&data);
            return rc;
        }

        RB_FREE(&data);

        break;
    }
    default:
        return RB_INVALID_ARG;
    }

    RB_FREE(&key);

    return RB_OK;
}
