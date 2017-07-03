#ifndef RB_TESTCOMMON_H_
#define RB_TESTCOMMON_H_

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define ASSERT(val) \
    do { \
        if((!(val))) { \
            RBLE("Assertion failure: " # val); \
            return -1; \
        } \
    }while(0)

#define ASSERT_NOT_NULL(val) ASSERT(val != NULL)

#define ASSERT_EQUAL(expected, actual) ASSERT(expected == actual)

#define ASSERT_NOT_EQUAL(a, b) ASSERT(a != b)

#define ASSERT_NULL(val) val == NULL

#endif
