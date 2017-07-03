/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/Log.h>
#include <rb/Utils.h>

#include <stdlib.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestUtils"

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testUtils() {
    int rc;

    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    {
        // Test Rb_Utils_growAppend
        uint32_t baseSize = 1;
        char* base = (char*) RB_MALLOC(baseSize);
        base[0] = 0;
        RBLI("base string '%s' [%d]", base, baseSize);

        Rb_Utils_growAppend(&base, baseSize, &baseSize, "[First append]");
        ASSERT_EQUAL(strlen(base) + 1, baseSize);
        RBLI("appended string '%s' [%d]", base, baseSize);

        Rb_Utils_growAppend(&base, baseSize, &baseSize, " [Second append]");
        ASSERT_EQUAL(strlen(base) + 1, baseSize);
        RBLI("appended string '%s' [%d]", base, baseSize);

        RB_FREE(&base);
        baseSize = 0;
    }

    {
        // Test Rb_Utils_print
        char* str = Rb_Utils_print("Test int=%d float=%.2f string='%s'", 42,
                3.14, "TEST");
        ASSERT_NOT_EQUAL(str, NULL);

        RBLI("Printed string: '%s'", str);

        RB_FREE(&str);
        str = NULL;
    }

    return 0;
}
