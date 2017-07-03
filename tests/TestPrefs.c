/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/Prefs.h>
#include <rb/Log.h>
#include <rb/FileStream.h>
#include <rb/Utils.h>

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
    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    int32_t rc;

    Rb_PrefsHandle prefs = Rb_Prefs_new(NULL);
    ASSERT_NOT_NULL(prefs);

    // Int32 test
    int32_t int32Val;
    rc = Rb_Prefs_putInt32(prefs, INT32_KEY, INT32_VAL);
    ASSERT_EQUAL(RB_OK, rc);

    rc = Rb_Prefs_getInt32(prefs, INT32_KEY, &int32Val);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(INT32_VAL, int32Val);

    // Int64 test
    int64_t int64Val;
    rc = Rb_Prefs_putInt64(prefs, INT64_KEY, INT64_VAL);
    ASSERT_EQUAL(RB_OK, rc);

    rc = Rb_Prefs_getInt64(prefs, INT64_KEY, &int64Val);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(INT64_VAL, int64Val);

    // float test
    float floatVal;
    rc = Rb_Prefs_putFloat(prefs, FLOAT_KEY, FLOAT_VAL);
    ASSERT_EQUAL(RB_OK, rc);

    rc = Rb_Prefs_getFloat(prefs, FLOAT_KEY, &floatVal);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(FLOAT_VAL, floatVal);

    // String test
    char* stringVal;
    rc = Rb_Prefs_putString(prefs, STRING_KEY, STRING_VAL);
    ASSERT_EQUAL(RB_OK, rc);

    rc = Rb_Prefs_getString(prefs, STRING_KEY, &stringVal);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(0, strcmp(STRING_VAL, stringVal));

    RB_FREE(&stringVal);

    // Blob test
    int32_t i;
    uint8_t blob[BLOB_SIZE];
    for (i = 0; i < BLOB_SIZE; i++) {
        blob[i] = i % 0xFF;
    }
    rc = Rb_Prefs_putBlob(prefs, BLOB_KEY, blob, BLOB_SIZE);
    ASSERT_EQUAL(RB_OK, rc);

    uint8_t* outBlob;
    uint32_t outBlobSize;

    rc = Rb_Prefs_getBlob(prefs, BLOB_KEY, (void**) &outBlob, &outBlobSize);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(BLOB_SIZE, outBlobSize);
    ASSERT_EQUAL(0, memcmp(outBlob, blob, outBlobSize));

    RB_FREE(&outBlob);

    ASSERT_EQUAL(NUM_TEST_VALUES, Rb_Prefs_getNumEntries(prefs));

    rc = Rb_Prefs_saveFile(prefs, TEST_FILE_PATH);
    ASSERT_EQUAL(RB_OK, rc);

    rc = Rb_Prefs_free(&prefs);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(prefs, NULL);

    prefs = Rb_Prefs_new(NULL);
    ASSERT_NOT_NULL(prefs);

    rc = Rb_Prefs_loadFile(prefs, TEST_FILE_PATH);
    ASSERT_EQUAL(RB_OK, rc);

    ASSERT_EQUAL(NUM_TEST_VALUES, Rb_Prefs_getNumEntries(prefs));

    rc = Rb_Prefs_getInt32(prefs, INT32_KEY, &int32Val);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(INT32_VAL, int32Val);

    rc = Rb_Prefs_getInt64(prefs, INT64_KEY, &int64Val);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(INT64_VAL, int64Val);

    rc = Rb_Prefs_getFloat(prefs, FLOAT_KEY, &floatVal);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(FLOAT_VAL, floatVal);

    rc = Rb_Prefs_getBlob(prefs, BLOB_KEY, (void**) &outBlob, &outBlobSize);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(BLOB_SIZE, outBlobSize);
    ASSERT_EQUAL(0, memcmp(outBlob, blob, outBlobSize));

    RB_FREE(&outBlob);

    rc = Rb_Prefs_getString(prefs, STRING_KEY, &stringVal);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(0, strcmp(stringVal, STRING_VAL));

    RB_FREE(&stringVal);

    rc = Rb_Prefs_free(&prefs);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(prefs, NULL);

    system("rm " TEST_FILE_PATH);

    return 0;
}
