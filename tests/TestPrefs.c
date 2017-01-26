/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/Prefs.h>
#include <rb/Log.h>
#include <rb/FileStream.h>

#include <stdlib.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestPrefs"

#define INT32_KEY "int32_key"
#define INT32_VAL ( 42 )

#define INT64_KEY "int64_key"
#define INT64_VAL ( 123812738123L )

#define FLOAT_KEY "float_key"
#define FLOAT_VAL ( 3.14f )

#define STRING_KEY "string_key"
#define STRING_VAL "string value"

#define BLOB_KEY "blob_key"
#define BLOB_SIZE ( 32 )

#define NUM_TEST_VALUES ( 5 )

#ifdef ANDROID
#define TEST_FILE_PATH "/data/test_prefs_file.bin"
#else
#define TEST_FILE_PATH "test_prefs_file.bin"
#endif


/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testPrefs() {
    if(!RB_CHECK_VERSION){
        RBLE("Invalid binary version");
        return -1;
    }

    int32_t rc;

    Rb_PrefsHandle prefs = Rb_Prefs_new(NULL);
    if(!prefs){
        RBLE("Rb_Prefs_new failed");
        return -1;
    }

    // Int32 test
    int32_t int32Val;
    rc = Rb_Prefs_putInt32(prefs, INT32_KEY, INT32_VAL);
    if(rc != RB_OK){
        RBLE("Rb_Prefs_putInt32 failed");
        return -1;
    }

    rc = Rb_Prefs_getInt32(prefs, INT32_KEY, &int32Val);
    if(rc != RB_OK || int32Val != INT32_VAL){
        RBLE("Rb_Prefs_getInt32 failed");
        return -1;
    }

    // Int64 test
    int64_t int64Val;
    rc = Rb_Prefs_putInt64(prefs, INT64_KEY, INT64_VAL);
    if(rc != RB_OK){
        RBLE("Rb_Prefs_putInt64 failed");
        return -1;
    }

    rc = Rb_Prefs_getInt64(prefs, INT64_KEY, &int64Val);
    if(rc != RB_OK || int64Val != INT64_VAL){
        RBLE("Rb_Prefs_getInt64 failed");
        return -1;
    }

    // float test
    float floatVal;
    rc = Rb_Prefs_putFloat(prefs, FLOAT_KEY, FLOAT_VAL);
    if(rc != RB_OK){
        RBLE("Rb_Prefs_putFloat failed");
        return -1;
    }

    rc = Rb_Prefs_getFloat(prefs, FLOAT_KEY, &floatVal);
    if(rc != RB_OK || floatVal != FLOAT_VAL){
        RBLE("Rb_Prefs_getFLoat failed");
        return -1;
    }

    // String test
    char* stringVal;
    rc = Rb_Prefs_putString(prefs, STRING_KEY, STRING_VAL);
    if(rc != RB_OK){
        RBLE("Rb_Prefs_putString failed");
        return -1;
    }

    rc = Rb_Prefs_getString(prefs, STRING_KEY, &stringVal);
    if(rc != RB_OK || strcmp(stringVal, STRING_VAL) != 0){
        RBLE("Rb_Prefs_getString failed");
        return -1;
    }

    free(stringVal);

    // Blob test
    int32_t i;
    uint8_t blob[BLOB_SIZE];
    for(i=0; i<BLOB_SIZE; i++){
        blob[i] = i % 0xFF;
    }
    rc = Rb_Prefs_putBlob(prefs, BLOB_KEY, blob, BLOB_SIZE);
    if(rc != RB_OK){
        RBLE("Rb_Prefs_putBlob failed");
        return -1;
    }

    uint8_t* outBlob;
    uint32_t outBlobSize;

    rc = Rb_Prefs_getBlob(prefs, BLOB_KEY, (void**)&outBlob, &outBlobSize);
    if(rc != RB_OK || outBlobSize != BLOB_SIZE || memcmp(outBlob, blob, outBlobSize)){
        RBLE("Rb_Prefs_getBlob failed");
        return -1;
    }

    free(outBlob);

    if(Rb_Prefs_getNumEntries(prefs) != NUM_TEST_VALUES){
        RBLE("Rb_Prefs_getNumEntries failed");
        return -1;
    }

    rc = Rb_Prefs_saveFile(prefs, TEST_FILE_PATH);
    if(rc != RB_OK){
        RBLE("Rb_Prefs_save failed");
        return -1;
    }

    rc = Rb_Prefs_free(&prefs);
    if(rc != RB_OK || prefs){
        RBLE("Rb_Prefs_free failed");
        return -1;
    }

    prefs = Rb_Prefs_new(NULL);
    if(!prefs){
        RBLE("Rb_Prefs_new failed");
        return -1;
    }

    rc = Rb_Prefs_loadFile(prefs, TEST_FILE_PATH);
    if(rc != RB_OK){
        RBLE("Rb_Prefs_load failed");
        return -1;
    }

    if(Rb_Prefs_getNumEntries(prefs) != NUM_TEST_VALUES){
        RBLE("Rb_Prefs_getNumEntries failed %d", Rb_Prefs_getNumEntries(prefs));
        return -1;
    }

    rc = Rb_Prefs_getInt32(prefs, INT32_KEY, &int32Val);
    if (rc != RB_OK || int32Val != INT32_VAL) {
        RBLE("Rb_Prefs_getInt32 failed");
        return -1;
    }

    rc = Rb_Prefs_getInt64(prefs, INT64_KEY, &int64Val);
    if (rc != RB_OK || int64Val != INT64_VAL) {
        RBLE("Rb_Prefs_getInt64 failed");
        return -1;
    }

    rc = Rb_Prefs_getFloat(prefs, FLOAT_KEY, &floatVal);
    if (rc != RB_OK || floatVal != FLOAT_VAL) {
        RBLE("Rb_Prefs_getFLoat failed");
        return -1;
    }

    rc = Rb_Prefs_getBlob(prefs, BLOB_KEY, (void**) &outBlob, &outBlobSize);
    if (rc != RB_OK || outBlobSize != BLOB_SIZE
            || memcmp(outBlob, blob, outBlobSize)) {
        RBLE("Rb_Prefs_getBlob failed");
        return -1;
    }
    free(outBlob);

    rc = Rb_Prefs_getString(prefs, STRING_KEY, &stringVal);
    if (rc != RB_OK || strcmp(stringVal, STRING_VAL) != 0) {
        RBLE("Rb_Prefs_getString failed");
        return -1;
    }

    free(stringVal);

    rc = Rb_Prefs_free(&prefs);
    if (rc != RB_OK || prefs) {
        RBLE("Rb_Prefs_free failed");
        return -1;
    }

    system("rm " TEST_FILE_PATH);

    return 0;
}
