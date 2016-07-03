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

    rc = Rb_List_free(&list);
    if(rc != RB_OK || list){
        RBLE("Rb_List_free failed");
        return -1;
    }

    return 0;
}
