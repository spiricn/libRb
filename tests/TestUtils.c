/*******************************************************/
/*              Includes                               */
/*******************************************************/

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
    if (!RB_CHECK_VERSION) {
        RBLE("Invalid binary version");
        return -1;
    }

    {
        // Test Rb_Utils_growAppend
        uint32_t baseSize = 1;
        char* base = (char*) malloc(baseSize);
        base[0] = 0;
        RBLI("base string '%s' [%d]", base, baseSize);

        Rb_Utils_growAppend(&base, baseSize, &baseSize, "[First append]");
        RBLI("appended string '%s' [%d]", base, baseSize);
        if (baseSize != strlen(base) + 1) {
            RBLE("Unexpected size %d", baseSize);
            return -1;
        }

        Rb_Utils_growAppend(&base, baseSize, &baseSize, " [Second append]");
        RBLI("appended string '%s' [%d]", base, baseSize);
        if (baseSize != strlen(base) + 1) {
            RBLE("Unexpected size %d", baseSize);
            return -1;
        }

        free(base);
        base = NULL;
        baseSize = 0;
    }

    {
        // Test Rb_Utils_print
        char* str = Rb_Utils_print("Test int=%d float=%.2f string='%s'", 42,
                3.14, "TEST");

        RBLI("Printed string: '%s'", str);

        free(str);
        str = NULL;
    }

    return 0;
}
