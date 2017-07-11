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

// Integer
#define ASSERT_EQUAL_INT32(expected, actual) \
    do { \
        if((expected) != (actual)){ \
            RBLE("Assertion failure: %d != %d", (expected), (actual)); \
            return -1; \
        } \
    } while(0)

#define ASSERT_EQUAL_FLOAT(expected, actual) \
    do { \
        if((expected) != (actual)){ \
            RBLE("Assertion failure: %f != %f", (expected), (actual)); \
            return -1; \
        } \
    } while(0)

#define ASSERT_EQUAL_STR(expected, actual) \
    do { \
    if(strcmp((expected), (actual)) != 0) { \
        RBLE("Assertion failure: '%s' != '%s'", (expected), (actual)); \
        return -1; \
    } \
    }while(0)

#define ASSERT_NOT_EQUAL(a, b) ASSERT(a != b)

#define ASSERT_NULL(val) val == NULL

#endif
