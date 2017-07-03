/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/List.h>
#include <rb/Log.h>
#include <rb/Utils.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestList"

#define TEST_STR "test string"

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
    int32_t testData1;
    char testData2;
    char testData3[64];
} ListElement;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static int32_t testCompareFnc(Rb_ListHandle handle, void* elem1, void* elem2);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testList() {
    int32_t rc;
    ListElement e1;

    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    Rb_ListHandle list = Rb_List_new(sizeof(ListElement));
    ASSERT_NOT_NULL(list);

    ASSERT_EQUAL(0, Rb_List_getSize(list));

    const int32_t kNUM_TEST_ELEMS = 64;

    // Test add
    int32_t i;
    for(i = 0; i < kNUM_TEST_ELEMS; i++) {
        memset(&e1, 0x00, sizeof(ListElement));
        e1.testData1 = i;
        e1.testData2 = i % 0xFF;
        memset(e1.testData3, 0xAA, sizeof(e1.testData3));

        rc = Rb_List_add(list, &e1);
        ASSERT_EQUAL(RB_OK, rc);

        ASSERT_EQUAL(i + 1, Rb_List_getSize(list));

        ListElement e2;
        rc = Rb_List_get(list, i, &e2);
        ASSERT_EQUAL(RB_OK, rc);

        ASSERT_EQUAL(0, memcmp(&e1, &e2, sizeof(ListElement)));
    }

    // Test insert
    {
        for(i = 0; i < 3; i++) {
            int32_t kTEST_NUM = 56 * i;

            int32_t kINSERT_INDEX = -1;

            if(i == 0) {
                // Beginning insertion
                kINSERT_INDEX = 0;
            } else if(i == 1) {
                // Middle insertion
                kINSERT_INDEX = Rb_List_getSize(list) / 2;
            } else {
                // End insertion
                kINSERT_INDEX = Rb_List_getSize(list);
            }

            ListElement insertEl = { kTEST_NUM, kTEST_NUM, TEST_STR };

            int32_t numElems = Rb_List_getSize(list);

            rc = Rb_List_insert(list, kINSERT_INDEX, &insertEl);
            ASSERT_EQUAL(RB_OK, rc);

            ASSERT_EQUAL(numElems + 1, Rb_List_getSize(list));

            memset(&insertEl, 0x00, sizeof(ListElement));

            rc = Rb_List_get(list, kINSERT_INDEX, &insertEl);
            ASSERT_EQUAL(RB_OK, rc);

            ASSERT_EQUAL(insertEl.testData1, kTEST_NUM);
            ASSERT_EQUAL(insertEl.testData2, kTEST_NUM);
            ASSERT_EQUAL(0, strcmp(insertEl.testData3, TEST_STR));
        }
    }

    rc = Rb_List_clear(list);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(0, Rb_List_getSize(list));

    // Test sorting
#define NUM_SORT_ELEMS 5
    static const int32_t kSORT_ELEMS[NUM_SORT_ELEMS] = { 5, 32, 1, -45, 150 };

    for(i = 0; i < NUM_SORT_ELEMS; i++) {
        ListElement e;
        e.testData1 = kSORT_ELEMS[i];

        rc = Rb_List_add(list, &e);
        ASSERT_EQUAL(RB_OK, rc);
    }

    rc = Rb_List_sort(list, testCompareFnc, eRB_SORT_ASCEND);
    ASSERT_EQUAL(RB_OK, rc);

    int32_t prevVal = 0;
    for(i = 0; i < Rb_List_getSize(list); i++) {
        ListElement e;
        rc = Rb_List_get(list, i, &e);
        ASSERT_EQUAL(RB_OK, rc);

        if(i != 0) {
            ASSERT(e.testData1 > prevVal);
        }

        prevVal = e.testData1;
    }

    rc = Rb_List_free(&list);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_NULL(list);

    return 0;
}

int32_t testCompareFnc(Rb_ListHandle handle, void* elem1, void* elem2) {
    RB_UNUSED(handle);

    int32_t v1 = ((ListElement*) elem1)->testData1;
    int32_t v2 = ((ListElement*) elem2)->testData1;

    return v1 < v2 ? -1 : v1 > v2 ? 1 : 0;
}
