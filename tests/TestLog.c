/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/Log.h>

#include <stdlib.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestLog"

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static void testLogCallback(const Rb_MessageInfo* info,
        const char* finalMessage, void* userData);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testLog() {
    RBL_FN_ENTER
    ;

    int rc;
    if (!RB_CHECK_VERSION) {
        RBLE("Invalid binary version");
        RBL_RETURN(-1);
    }

    Rb_LogOutputConfig config;

    rc = Rb_log_getOutputConfig(eRB_LOG_OUTPUT_CUSTOM, &config);
    if (rc != RB_OK) {
        RBLE("Rb_log_getOutputConfig failed");
        RBL_RETURN(-1);
    }

    config.enabled = true;
    config.data.custom.fnc = testLogCallback;
    config.data.custom.userData = (void*) 0xAAAAAAAA;

    rc = Rb_log_setOutputConfig(eRB_LOG_OUTPUT_CUSTOM, &config);
    if (rc != RB_OK) {
        RBLE("Rb_log_setOutputConfig failed");
        RBL_RETURN(-1);
    }

    RBLI("Test log !");

    config.enabled = false;
    rc = Rb_log_setOutputConfig(eRB_LOG_OUTPUT_CUSTOM, &config);
    if (rc != RB_OK) {
        RBLE("Rb_log_setOutputConfig failed");
        RBL_RETURN(-1);
    }

    RBLI("Test new line log:\n"
            "Line1\n"
            "\n\n\n"
            "Line2\n"
            "\n"
            "Line3");

    RBL_RETURN(0);
}

void testLogCallback(const Rb_MessageInfo* info, const char* finalMessage,
        void* userData) {
    printf("Custom log callback: info=%p, userdata=%p\nFinal message: '%s'\n",
            info, userData, finalMessage);
}
