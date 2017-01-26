#ifndef RB_PREFS_H_
#define RB_PREFS_H_

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "rb/Common.h"
#include "rb/IOStream.h"

#include <stdint.h>

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Rb_PrefsHandle;

typedef int32_t (*Rb_PrefsBackendSaveFnc)(Rb_PrefsHandle handle, const Rb_IOStream* stream);

typedef int32_t (*Rb_PrefsBackendLoadFnc)(Rb_PrefsHandle handle, const Rb_IOStream* stream);

typedef struct {
    Rb_PrefsBackendSaveFnc save;
    Rb_PrefsBackendLoadFnc load;
} Rb_PrefsBackend;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/**
 * Creates new preferences.
 *
 * @param[in] backend Preferences backend.
 * @return Valid preferences handle on success, NULL otherwise.
 */
Rb_PrefsHandle Rb_Prefs_new(const Rb_PrefsBackend* backend);

/**
 * Frees created preferences.
 *
 * @param[in] handle Pointer to a valid prefences handle.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_free(Rb_PrefsHandle* handle);

/**
 * Saves a 32-bit integer value.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[in] value Integer value.
 * @return RB_OK on success, negative value otherwise
 */
int32_t Rb_Prefs_putInt32(Rb_PrefsHandle handle, const char* key, int32_t value);

/**
 * Saves a 64-bit integer value.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[in] value Integer value.
 * @return RB_OK on success, negative value otherwise
 */
int32_t Rb_Prefs_putInt64(Rb_PrefsHandle handle, const char* key, int64_t value);

/**
 * Saves a float value.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[in] value Float value.
 * @return RB_OK on success, negative value otherwise
 */
int32_t Rb_Prefs_putFloat(Rb_PrefsHandle handle, const char* key, float value);

/**
 * Saves a string value.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[in] value String value.
 * @return RB_OK on success, negative value otherwise
 */
int32_t Rb_Prefs_putString(Rb_PrefsHandle handle, const char* key, const char* value);

/**
 * Saves binary data.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[in] data Data pointer.
 * @param[in] size Data length in bytes.
 * @return RB_OK on success, negative value otherwise
 */
int32_t Rb_Prefs_putBlob(Rb_PrefsHandle handle, const char* key, const void* data, uint32_t size);

/**
 * Gets previously saved 32-bit integer value.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[out] value Pointer to an integer where the result will be stored.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_getInt32(Rb_PrefsHandle handle, const char* key, int32_t* value);

/**
 * Gets previously saved 64-bit integer value.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[out] value Pointer to an integer where the result will be stored.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_getInt64(Rb_PrefsHandle handle, const char* key, int64_t* value);

/**
 * Gets previously saved float value.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[out] value Pointer to a float where the result will be stored.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_getFloat(Rb_PrefsHandle handle, const char* key, float* value);

/**
 * Gets previously saved string value.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[out] value Pointer to a string where the result will be stored. String is dynamically allocated, and should be freed by the user.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_getString(Rb_PrefsHandle handle, const char* key, char** value);

/**
 * Gets previously saved binary data value.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Value key.
 * @param[out] value Pointer to an integer where the result will be stored. Data is dynamically allocated, and should be freed by the user.
 * @param[out] size Pointer to an integer where the data size will be stored.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_getBlob(Rb_PrefsHandle handle, const char* key, void** data, uint32_t* size);

/**
 * Clears all saved values from the prefrences.
 *
 * @param[in] handle Valid preferences handle.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_clear(Rb_PrefsHandle handle);

/**
 * Gets number of saved entries.
 *
 * @param[in] handle Valid preferences handle.
 * @return Number of saved values, negative value otherwise.
 */
int32_t Rb_Prefs_getNumEntries(Rb_PrefsHandle handle);

/**
 * Gets string key of value with given index.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] index Index of given entry in range [0, Rb_Prefs_getNumEntries].
 * @param[out] key Entry key.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_getKey(Rb_PrefsHandle handle, uint32_t index, const char** key);

/**
 * Gets entry type with given key.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Entry key.
 * @param[out] type Entry type.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_getEntryType(Rb_PrefsHandle handle, const char* key, Rb_VariantType* type);

/**
 * Checks if preferences contain given key.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Entry key.
 * @return 0 if it doesn't contain key, 1 if it does, negative value on error.
 */
int32_t Rb_Prefs_contains(Rb_PrefsHandle handle, const char* key);

/**
 * Removes entry with given key.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] key Entry key.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_remove(Rb_PrefsHandle handle, const char* key);

/**
 * Loads entries from given stream. Clears all existing entries.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] stream Input stream.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_load(Rb_PrefsHandle handle, const Rb_IOStream* stream);

/**
 * Saves entries to given stream.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] stream Output stream.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_save(Rb_PrefsHandle handle, const Rb_IOStream* stream);

/**
 * Loads entries from given file location. Clears all existing entries.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] stream Input stream.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_loadFile(Rb_PrefsHandle handle, const char* filePath);

/**
 * Saves entries to given file location.
 *
 * @param[in] handle Valid preferences handle.
 * @param[in] stream Output stream.
 * @return RB_OK on success, negative value otherwise.
 */
int32_t Rb_Prefs_saveFile(Rb_PrefsHandle handle, const char* filePath);

#ifdef __cplusplus
}
#endif

#endif
