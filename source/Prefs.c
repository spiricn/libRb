/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Prefs.h"
#include "rb/Common.h"
#include "rb/List.h"
#include "rb/priv/PrefsPriv.h"
#include "rb/priv/PrefsBackend.h"
#include "rb/FileStream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define PREFS_MAGIC ( 0x565634A5 )


/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static PrefsContext* PrefsPriv_getContext(Rb_PrefsHandle handle);
static int32_t PrefsPriv_add(PrefsContext* prefs, const char* key, const Variant* var);
static PrefEntry* PrefsPriv_get(PrefsContext* prefs, const char* key);
static int32_t PrefsPriv_remove(PrefsContext* prefs, const char* key);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

Rb_PrefsHandle Rb_Prefs_new(const Rb_PrefsBackend* backend){
    PrefsContext* prefs = (PrefsContext*)calloc(1, sizeof(PrefsContext));

    prefs->magic = PREFS_MAGIC;

    if(backend){
        prefs->backend = *backend;
    }
    else{
        memset(&prefs->backend, 0x00, sizeof(Rb_PrefsBackend));

        prefs->backend.load = Rb_PrefsBackendLoad;
        prefs->backend.save = Rb_PrefsBackendSave;
    }

    prefs->entries = Rb_List_new(sizeof(PrefEntry*));
    if(prefs->entries == NULL){
        return NULL;
    }

    return (Rb_PrefsHandle)prefs;
}

int32_t Rb_Prefs_free(Rb_PrefsHandle* handle){
    // TODO
    int32_t rc;

    PrefsContext* prefs = PrefsPriv_getContext(*handle);
    if(prefs == NULL) {
        return RB_INVALID_ARG;
    }

    rc = Rb_List_free(&prefs->entries);
    if(rc != RB_OK){
        return rc;
    }

    free(prefs);
    *handle = NULL;

    return RB_OK;
}

int32_t Rb_Prefs_putInt32(Rb_PrefsHandle handle, const char* key, int32_t value){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL){
        return RB_INVALID_ARG;
    }

    int32_t rc;

    Variant var;

    var.type = eRB_VAR_TYPE_INT32;
    var.val.int32Val = value;

    return PrefsPriv_add(prefs, key, &var);
}

int32_t Rb_Prefs_putInt64(Rb_PrefsHandle handle, const char* key, int64_t value){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if (prefs == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t rc;

    Variant var;

    var.type = eRB_VAR_TYPE_INT64;
    var.val.int64Val = value;

    return PrefsPriv_add(prefs, key, &var);
}

int32_t Rb_Prefs_putFloat(Rb_PrefsHandle handle, const char* key, float value){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if (prefs == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t rc;

    Variant var;

    var.type = eRB_VAR_TYPE_FLOAT;
    var.val.floatVal = value;

    return PrefsPriv_add(prefs, key, &var);
}

int32_t Rb_Prefs_putString(Rb_PrefsHandle handle, const char* key, const char* value){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if (prefs == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t rc;

    Variant var;

    var.type = eRB_VAR_TYPE_STRING;
    var.val.stringVal = (char*)malloc(strlen(value) + 1);
    strcpy(var.val.stringVal, value);

    return PrefsPriv_add(prefs, key, &var);
}

int32_t Rb_Prefs_putBlob(Rb_PrefsHandle handle, const char* key, const void* data, uint32_t size){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if (prefs == NULL) {
        return RB_INVALID_ARG;
    }

    int32_t rc;

    Variant var;

    var.type = eRB_VAR_TYPE_BLOB;
    var.val.blobVal.size = size;
    var.val.blobVal.data = malloc(size);
    memcpy(var.val.blobVal.data, data, size);

    return PrefsPriv_add(prefs, key, &var);
}

int32_t Rb_Prefs_getInt32(Rb_PrefsHandle handle, const char* key, int32_t* value){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL || value == NULL){
        return RB_INVALID_ARG;
    }

    PrefEntry* entry = PrefsPriv_get(prefs, key);
    if(entry == NULL){
        return RB_INVALID_ARG;
    }

    if(entry->value.type != eRB_VAR_TYPE_INT32){
        return RB_INVALID_ARG;
    }

    *value = entry->value.val.int32Val;

    return RB_OK;
}

int32_t Rb_Prefs_getInt64(Rb_PrefsHandle handle, const char* key, int64_t* value){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL || value == NULL){
        return RB_INVALID_ARG;
    }

    PrefEntry* entry = PrefsPriv_get(prefs, key);
    if(entry == NULL){
        return RB_INVALID_ARG;
    }

    if(entry->value.type != eRB_VAR_TYPE_INT64){
        return RB_INVALID_ARG;
    }

    *value = entry->value.val.int64Val;

    return RB_OK;
}

int32_t Rb_Prefs_getFloat(Rb_PrefsHandle handle, const char* key, float* value){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL || value == NULL){
        return RB_INVALID_ARG;
    }

    PrefEntry* entry = PrefsPriv_get(prefs, key);
    if(entry == NULL){
        return RB_INVALID_ARG;
    }

    if(entry->value.type != eRB_VAR_TYPE_FLOAT){
        return RB_INVALID_ARG;
    }

    *value = entry->value.val.floatVal;

    return RB_OK;
}

int32_t Rb_Prefs_getString(Rb_PrefsHandle handle, const char* key, char** value){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL || value == NULL){
        return RB_INVALID_ARG;
    }

    PrefEntry* entry = PrefsPriv_get(prefs, key);
    if(entry == NULL){
        return RB_INVALID_ARG;
    }

    if(entry->value.type != eRB_VAR_TYPE_STRING){
        return RB_INVALID_ARG;
    }

    *value = (char*)malloc(strlen(entry->value.val.stringVal) + 1);
    strcpy(*value, entry->value.val.stringVal);

    return RB_OK;
}

int32_t Rb_Prefs_getBlob(Rb_PrefsHandle handle, const char* key, void** data, uint32_t* size){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL || data == NULL || size == NULL){
        return RB_INVALID_ARG;
    }

    PrefEntry* entry = PrefsPriv_get(prefs, key);
    if(entry == NULL){
        return RB_INVALID_ARG;
    }

    if(entry->value.type != eRB_VAR_TYPE_BLOB){
        return RB_INVALID_ARG;
    }

    *size = entry->value.val.blobVal.size;
    *data = malloc(*size);
    memcpy(*data, entry->value.val.blobVal.data, *size);

    return RB_OK;
}

int32_t Rb_Prefs_clear(Rb_PrefsHandle handle){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL){
        return RB_INVALID_ARG;
    }

    int32_t rc;

    while(Rb_List_getSize(prefs->entries)){
        PrefEntry* entry;

        rc = Rb_List_get(prefs->entries, 0, &entry);
        if(rc != RB_OK){
            return rc;
        }

        rc = PrefsPriv_remove(prefs, entry->key);
        if(rc != RB_OK){
            return rc;
        }
    }

    return RB_OK;
}

int32_t Rb_Prefs_getNumEntries(Rb_PrefsHandle handle){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL){
        return RB_INVALID_ARG;
    }

    return Rb_List_getSize(prefs->entries);
}

int32_t Rb_Prefs_getKey(Rb_PrefsHandle handle, uint32_t index, const char** key){
    int32_t rc;

    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL){
        return RB_INVALID_ARG;
    }

    if((int32_t)index >= Rb_List_getSize(prefs->entries)){
        return RB_INVALID_ARG;
    }

    PrefEntry* entry = NULL;
    rc = Rb_List_get(prefs->entries, index, &entry);
    if (rc != RB_OK) {
        return RB_INVALID_ARG;
    }

    *key = entry->key;

    return RB_OK;
}

int32_t Rb_Prefs_getEntryType(Rb_PrefsHandle handle, const char* key, Rb_VariantType* type){
    int32_t rc;

    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if (prefs == NULL) {
        return RB_INVALID_ARG;
    }

    PrefEntry* entry = PrefsPriv_get(prefs, key);
    if(entry == NULL){
        return RB_INVALID_ARG;
    }

    *type = entry->value.type;

    return RB_OK;
}

int32_t Rb_Prefs_contains(Rb_PrefsHandle handle, const char* key){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL){
        return RB_INVALID_ARG;
    }

    return PrefsPriv_get(prefs, key) != NULL;
}

int32_t Rb_Prefs_remove(Rb_PrefsHandle handle, const char* key){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL){
        return RB_INVALID_ARG;
    }

    return PrefsPriv_remove(prefs, key);
}

int32_t PrefsPriv_add(PrefsContext* prefs, const char* key, const Variant* var){
    if(PrefsPriv_get(prefs, key)){
        return RB_INVALID_ARG;
    }

    PrefEntry* entry = (PrefEntry*)calloc(1, sizeof(PrefEntry));

    entry->key = (char*)malloc(strlen(key) + 1);
    strcpy(entry->key, key);

    memcpy(&entry->value, var, sizeof(Variant));

    return Rb_List_add(prefs->entries, &entry);
}


PrefEntry* PrefsPriv_get(PrefsContext* prefs, const char* key){
    int32_t i;
    int32_t rc;

    for(i=0; i<Rb_List_getSize(prefs->entries); i++){
        PrefEntry* entry = NULL;
        rc = Rb_List_get(prefs->entries, i, &entry);
        if(rc != RB_OK){
            return NULL;
        }

        if(strcmp(key, entry->key) == 0){
            return entry;
        }
    }

    return NULL;
}

int32_t PrefsPriv_remove(PrefsContext* prefs, const char* key){
    int32_t i;
    int32_t rc;

    for (i = 0; i < Rb_List_getSize(prefs->entries); i++) {
        PrefEntry* entry = NULL;
        rc = Rb_List_get(prefs->entries, i, &entry);
        if (rc != RB_OK) {
            return rc;
        }

        if(strcmp(key, entry->key) == 0){
            rc = Rb_List_remove(prefs->entries, i);
            if(rc != RB_OK){
                return rc;
            }

            if(entry->value.type == eRB_VAR_TYPE_BLOB){
                free(entry->value.val.blobVal.data);
            }
            else if(entry->value.type == eRB_VAR_TYPE_STRING){
                free(entry->value.val.stringVal);
            }

            free(entry->key);
            free(entry);
        }
    }

    return RB_OK;
}


int32_t Rb_Prefs_save(Rb_PrefsHandle handle, const Rb_IOStream* stream){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if(prefs == NULL){
        return RB_INVALID_ARG;
    }

    return prefs->backend.save(handle, stream);
}

int32_t Rb_Prefs_load(Rb_PrefsHandle handle, const Rb_IOStream* stream){
    PrefsContext* prefs = PrefsPriv_getContext(handle);
    if (prefs == NULL) {
        return RB_INVALID_ARG;
    }

    return prefs->backend.load(handle, stream);
}

PrefsContext* PrefsPriv_getContext(Rb_PrefsHandle handle){
    if(handle == NULL) {
        return NULL;
    }

    PrefsContext* prefs = (PrefsContext*)handle;
    if(prefs->magic != PREFS_MAGIC) {
        return NULL;
    }

    return prefs;
}

int32_t Rb_Prefs_loadFile(Rb_PrefsHandle handle, const char* filePath) {
    int32_t rc = RB_OK;
    Rb_IOStream stream;

    rc = Rb_FileStream_getApi(&stream.api);
    if (rc != RB_OK) {
        return rc;
    }

    rc = stream.api.open(filePath, eRB_IO_MODE_READ, &stream.handle);
    if (rc != RB_OK) {
        return rc;
    }

    rc = Rb_Prefs_load(handle, &stream);
    if (rc != RB_OK) {
        return rc;
    }

    rc = stream.api.close(&stream.handle);
    if (rc != RB_OK) {
        return rc;
    }

    return rc;
}

int32_t Rb_Prefs_saveFile(Rb_PrefsHandle handle, const char* filePath) {
    int32_t rc = RB_OK;
    Rb_IOStream stream;

    rc = Rb_FileStream_getApi(&stream.api);
    if (rc != RB_OK) {
        return rc;
    }

    rc = stream.api.open(filePath, eRB_IO_MODE_WRITE, &stream.handle);
    if (rc != RB_OK) {
        return rc;
    }

    rc = Rb_Prefs_save(handle, &stream);
    if (rc != RB_OK) {
        return rc;
    }

    rc = stream.api.close(&stream.handle);
    if (rc != RB_OK) {
        return rc;
    }

    return rc;
}
