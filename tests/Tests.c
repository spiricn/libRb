/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/Log.h>
#include <rb/Common.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestBuffer"
#define ADD_TEST(x) {x, #x},

#ifdef ANDROID
#define LOG_FILE "/data/rb_log.txt"
#else
#define LOG_FILE "rb_log.txt"
#endif

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef int (*testFnc)();
;

typedef struct {
    testFnc fnc;
    char* name;
} TestEntry;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

extern int testBuffer();
extern int testCBuffer();
extern int testConcurrency();
extern int testArray();
extern int testMessageBox();
extern int testList();
extern int testPrefs();
extern int testTimer();
extern int testLog();

static int runTests();
static int setupLogging();
/********************************************************/
/*                 Local Module Variables (MODULE)      */
/********************************************************/

static const TestEntry gTests[] = {
ADD_TEST(testBuffer)
ADD_TEST(testCBuffer)
ADD_TEST(testConcurrency)
ADD_TEST(testArray)
ADD_TEST(testMessageBox)
ADD_TEST(testList)
ADD_TEST(testPrefs)
ADD_TEST(testTimer)
ADD_TEST(testLog) };

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int main() {
    return runTests();
}

int runTests() {
    int32_t i;
    int32_t rc;
    int32_t numFailed = 0;
    int32_t numPassed = 0;

    if (setupLogging()) {
        return -1;
    }

    const int numTests = sizeof(gTests) / sizeof(TestEntry);

    RBLI("Running %d test(s) ..", numTests);
    RBLI("Logging results to '" LOG_FILE "' ..");


    for (i = 0; i < numTests; i++) {
        const TestEntry* test = &gTests[i];

        RBLI("-------------------------------------");
        RBLI("Running test '%s' (%d/%d)", test->name, i + 1, numTests);

        int32_t rc = test->fnc();

        if (rc) {
            RBLE("Test failed: %d", rc);
            ++numFailed;
        } else {
            RBLI("Test OK");
            ++numPassed;
        }
    }

    RBLI("-------------------------------------");
    RBLE("Tests failed: %d ( %.2f%% )", numFailed,
            ((float )numFailed / (float )numTests) * 100.0f);
    RBLI("Tests passed: %d ( %.2f%% )", numPassed,
            ((float )numPassed / (float )numTests) * 100.0f);

    return numFailed ? -1 : 0;
}

int setupLogging() {
    Rb_LogOutputConfig logOutputConfig;

    // Add log file output
    Rb_log_getOutputConfig(eRB_LOG_OUTPUT_FILE, &logOutputConfig);
    logOutputConfig.enabled = true;
    logOutputConfig.data.file.output = fopen(LOG_FILE, "wb");
    Rb_log_setOutputConfig(eRB_LOG_OUTPUT_FILE, &logOutputConfig);

    // On Android enable STDOUT (disabled by default)
#ifdef ANDROID
    Rb_log_getOutputConfig(eRB_LOG_OUTPUT_STDOUT, &logOutputConfig);
    logOutputConfig.enabled = true;
    Rb_log_setOutputConfig(eRB_LOG_OUTPUT_STDOUT, &logOutputConfig);
#endif

    return 0;
}
