/*******************************************************/
/*              Includes                               */
/*******************************************************/

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

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

typedef struct {
    int32_t num;
    char string[64];
} VectorElement;

int testVector() {
    int32_t rc;

    if (!RB_CHECK_VERSION) {
        RBLE("Invalid binary version");
        return -1;
    }

    // Create a vector
    Rb_VectorHandle vec = Rb_Vector_new(sizeof(VectorElement));
    if (vec == NULL) {
        RBLE("Rb_Vector_new failed");
        return -1;
    }

    // Insert a single element
    VectorElement el1 = { 42, TEST_STR };
    rc = Rb_Vector_add(vec, &el1);
    if (rc != RB_OK) {
        RBLE("Rb_Vector_add failed");
        return -1;
    }

    // Check the number of elements
    if (Rb_Vector_getNumElements(vec) != 1) {
        RBLE("Rb_Vector_getNumElements failed");
        return -1;
    }

    // Validate data pointer
    void* data = Rb_Vector_getData(vec);
    if (data == NULL) {
        RBLE("Rb_Vector_getData failed");
        return -1;
    }

    // There should at least be enough data to store a single element
    if (Rb_Vector_getSize(vec) < sizeof(VectorElement)) {
        RBLE("Rb_Vector_getSize failed");
        return -1;
    }

    // Check if the data is OK
    if (memcmp(data, &el1, sizeof(VectorElement)) != 0) {
        RBLE("Invalid data");
        return -1;
    }

    // Test clear
    if (Rb_Vector_clear(vec) != RB_OK || Rb_Vector_getNumElements(vec) > 0) {
        RBLE("Rb_Vector_clear failed");
        return -1;
    }

    // Check resizing (should add more elements than the internal size increase step)
    int32_t i;
    for (i = 0; i < NUM_TEST_ELEMENTS; i++) {
        VectorElement el = { i, TEST_STR };

        if (Rb_Vector_add(vec, &el) != RB_OK) {
            RBLE("Rb_Vector_add failed");
        }
    }

    if (Rb_Vector_getNumElements(vec) != NUM_TEST_ELEMENTS) {
        RBLE("Invalid number of elements");
        return -1;
    }

    if (Rb_Vector_getSize(vec) < NUM_TEST_ELEMENTS * sizeof(VectorElement)) {
        RBLE("Invalid vector size");
        return -1;
    }

    for (i = 0; i < NUM_TEST_ELEMENTS; i++) {
        VectorElement* el = Rb_Vector_get(vec, i);
        if (el == NULL) {
            RBLE("Rb_Vector_get failed");
            return -1;
        }

        if (el->num != i || strcmp(el->string, TEST_STR) != 0) {
            RBLE("Invalid element");
            return -1;
        }
    }

    rc = Rb_Vector_free(&vec);
    if (rc != RB_OK) {
        RBLE("Rb_Vector_free failed");
        return -1;
    }

    if (vec) {
        RBLE("Rb_free failed");
        return -1;
    }

    return 0;
}
