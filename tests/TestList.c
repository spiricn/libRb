/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/List.h>
#include <rb/Log.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestList"

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

    if(!RB_CHECK_VERSION){
        RBLE("Invalid binary version");
        return -1;
    }

    Rb_ListHandle list = Rb_List_new(sizeof(ListElement));
    if(!list){
        RBLE("Rb_List_new failed");
        return -1;
    }

    if(Rb_List_getSize(list) != 0){
        RBLE("Rb_List_getSize failed");
        return -1;
    }

    const int32_t kNUM_TEST_ELEMS = 64;

    // Test insertion
    int32_t i;
    for(i=0; i<kNUM_TEST_ELEMS; i++){
        memset(&e1, 0x00, sizeof(ListElement));
        e1.testData1 = i;
        e1.testData2 = i % 0xFF;
        memset(e1.testData3, 0xAA, sizeof(e1.testData3));

        rc = Rb_List_add(list, &e1);
        if(rc != RB_OK){
            RBLE("Rb_List_add failed");
            return -1;
        }

        if(Rb_List_getSize(list) != i+1){
            RBLE("Rb_List_getSize failed");
            return -1;
        }

        ListElement e2;
        rc = Rb_List_get(list, i, &e2);
        if(rc != RB_OK){
            RBLE("Rb_List_get failed");
            return -1;
        }

        if(memcmp(&e1, &e2, sizeof(ListElement))){
            RBLE("Rb_List_get failed");
            return -1;
        }
    }

    rc = Rb_List_clear(list);
    if(rc != RB_OK || Rb_List_getSize(list) != 0){
        RBLE("Rb_List_clear failed");
        return -1;
    }

    // Test sorting
    #define NUM_SORT_ELEMS 5
    static const int32_t kSORT_ELEMS[NUM_SORT_ELEMS] = { 5, 32, 1, -45, 150 };

    for(i=0; i<NUM_SORT_ELEMS; i++){
        ListElement e;
        e.testData1 = kSORT_ELEMS[i];

        rc = Rb_List_add(list, &e);
        if(rc != RB_OK){
            RBLE("Rb_List_add failed");
            return -1;
        }
    }

    rc = Rb_List_sort(list, testCompareFnc, eRB_SORT_ASCEND);
    if(rc != RB_OK){
        RBLE("Rb_List_sort failed");
        return -1;
    }

    int32_t prevVal = 0;
    for(i=0; i<Rb_List_getSize(list); i++){
        ListElement e;
        rc = Rb_List_get(list, i, &e);
        if(rc != RB_OK){
            RBLE("Rb_List_get failed");
            return -1;
        }

        if(i != 0 && e.testData1 < prevVal){
            RBLE("Rb_List_sort failed");
            return -1;
        }

        prevVal = e.testData1;
    }

    rc = Rb_List_free(&list);
    if(rc != RB_OK || list){
        RBLE("Rb_List_free failed");
        return -1;
    }

    return 0;
}


int32_t testCompareFnc(Rb_ListHandle handle, void* elem1, void* elem2){
    RB_UNUSED(handle);

    int32_t v1 = ((ListElement*)elem1)->testData1;
    int32_t v2 = ((ListElement*)elem2)->testData1;

    return v1 < v2 ? -1 : v1 > v2 ? 1 : 0;
}
