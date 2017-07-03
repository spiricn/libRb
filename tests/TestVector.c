/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/Vector.h>
#include <rb/Log.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestVector"

#define TEST_STR "VectorTestElement"
#define NUM_TEST_ELEMENTS ( 32 )
#define NUM_RANGE_ELEMTS ( 3 )

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

typedef struct {
    int32_t num;
    char string[64];
} VectorElement;

int testVector() {
    int32_t rc;

    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    // Create a vector
    Rb_VectorHandle vec = Rb_Vector_new(sizeof(VectorElement));
    ASSERT_NOT_NULL(vec);

    // Insert a single element
    VectorElement el1 = { 42, TEST_STR };
    rc = Rb_Vector_add(vec, &el1);
    ASSERT_EQUAL(RB_OK, rc);

    // Check the number of elements
    ASSERT_EQUAL(1, Rb_Vector_getNumElements(vec));

    // Validate data pointer
    void* data = Rb_Vector_getData(vec);
    ASSERT_NOT_EQUAL(data, NULL);

    // There should at least be enough data to store a single element
    ASSERT((int32_t )sizeof(VectorElement) >= Rb_Vector_getSize(vec));

    // Check if the data is OK
    ASSERT_EQUAL(0, memcmp(data, &el1, sizeof(VectorElement)));

    // Test clear
    rc = Rb_Vector_clear(vec);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(0, Rb_Vector_getNumElements(vec));

    // Check resizing (should add more elements than the internal size increase step)
    int32_t i;
    for (i = 0; i < NUM_TEST_ELEMENTS; i++) {
        VectorElement el = { i, TEST_STR };

        rc = Rb_Vector_add(vec, &el);
        ASSERT_EQUAL(RB_OK, rc);
    }

    ASSERT_EQUAL(NUM_TEST_ELEMENTS, Rb_Vector_getNumElements(vec));
    ASSERT(
            (NUM_TEST_ELEMENTS * (int32_t)sizeof(VectorElement)) >= Rb_Vector_getSize(vec));

    for (i = 0; i < NUM_TEST_ELEMENTS; i++) {
        VectorElement* el = Rb_Vector_get(vec, i);
        ASSERT_NOT_EQUAL(el, NULL);
        ASSERT_EQUAL(i, el->num);
        ASSERT_EQUAL(0, strcmp(el->string, TEST_STR));
    }

    // Test remove range
    rc = Rb_Vector_removeRange(vec, 1, 3);
    ASSERT_EQUAL(RB_OK, RB_OK);
    ASSERT_EQUAL(NUM_TEST_ELEMENTS - 3, Rb_Vector_getNumElements(vec));

    // Verify elements
    for (i = 0; i < NUM_TEST_ELEMENTS - 3; i++) {
        VectorElement* el = Rb_Vector_get(vec, i);
        ASSERT_NOT_EQUAL(el, NULL);
        ASSERT_EQUAL(0, strcmp(el->string, TEST_STR));

        if (i < 1) {
            ASSERT_EQUAL(i, el->num);
        } else {
            ASSERT_EQUAL(i + 3, el->num);
        }
    }

    // Test single remove
    rc = Rb_Vector_remove(vec, 0);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL((NUM_TEST_ELEMENTS - 3 - 1), Rb_Vector_getNumElements(vec));

    // Verify elements
    for (i = 0; i < NUM_TEST_ELEMENTS - 3 - 1; i++) {
        VectorElement* el = Rb_Vector_get(vec, i);
        ASSERT_NOT_EQUAL(el, NULL);

        ASSERT_EQUAL(0, strcmp(el->string, TEST_STR));
        ASSERT_EQUAL(i + 4, el->num);
    }

    int32_t numElems = Rb_Vector_getNumElements(vec);

    // Test insert
    VectorElement el = { 42, TEST_STR };
    rc = Rb_Vector_insert(vec, 1, &el);
    ASSERT_EQUAL(RB_OK, rc);

    ASSERT_EQUAL(numElems + 1, Rb_Vector_getNumElements(vec));

    VectorElement* insertedEl = Rb_Vector_get(vec, 1);
    ASSERT_NOT_EQUAL(insertedEl, NULL);
    ASSERT_EQUAL(42, insertedEl->num);
    ASSERT_EQUAL(0, strcmp(insertedEl->string, TEST_STR));

    // Test clear
    rc = Rb_Vector_clear(vec);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(0, Rb_Vector_getNumElements(vec));

    // Test addRange
    const VectorElement elems[NUM_RANGE_ELEMTS] = { { 0, TEST_STR }, { 1,
    TEST_STR }, { 2, TEST_STR } };
    rc = Rb_Vector_addRange(vec, &elems, NUM_RANGE_ELEMTS);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_EQUAL(NUM_RANGE_ELEMTS, Rb_Vector_getNumElements(vec));

    rc = Rb_Vector_free(&vec);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_NULL(vec);

    return 0;
}
