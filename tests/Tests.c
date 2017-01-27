/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/Log.h>
#include <rb/Common.h>

#include <stdlib.h>
#include <pthread.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestBuffer"
#define ADD_TEST(x) {test ## x, #x},
#define DECLARE_TEST(x) extern int test ## x ()
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

DECLARE_TEST(Buffer);
DECLARE_TEST(CBuffer);
DECLARE_TEST(Concurrency);
DECLARE_TEST(Array);
DECLARE_TEST(MessageBox);
DECLARE_TEST(List);
DECLARE_TEST(Prefs);
DECLARE_TEST(Timer);
DECLARE_TEST(Log);
DECLARE_TEST(Utils);

static int runTests();
static int setupLogging();
static void* testRunner(void* arg);

/********************************************************/
/*                 Local Module Variables (MODULE)      */
/********************************************************/

static const TestEntry gTests[] = {
ADD_TEST(Buffer)
ADD_TEST(CBuffer)
ADD_TEST(Concurrency)
ADD_TEST(Array)
ADD_TEST(MessageBox)
ADD_TEST(List)
ADD_TEST(Prefs)
ADD_TEST(Timer)
ADD_TEST(Log)
ADD_TEST(Utils) };

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int main(int argc, char* argv[]) {
    uint32_t i = 0;
    uint32_t j;

    if (setupLogging()) {
        return -1;
    }

    const uint32_t numTests = sizeof(gTests) / sizeof(TestEntry);

    TestEntry* entries = NULL;
    uint32_t numEntries = 0;

    if (argc > 1) {
        numEntries = argc - 1;
        entries = (TestEntry*) calloc(argc - 1, sizeof(TestEntry));

        for (i = 1; i < (uint32_t) argc; i++) {
            TestEntry* entry = NULL;
            char* testName = argv[i];

            for (j = 0; j < numTests; j++) {
                if (strcmp(gTests[j].name, testName) == 0) {
                    entry = (TestEntry*) &gTests[j];
                    break;
                }
            }

            if (entry == NULL) {
                RBLE("No test named '%s'", testName);
                return -1;
            }

            memcpy(&entries[i - 1], entry, sizeof(TestEntry));
        }
    } else {
        numEntries = numTests;
        entries = (TestEntry*) &gTests;
    }

    return runTests(entries, numEntries);
}

int runTests(const TestEntry* entries, uint32_t numTests) {
    uint32_t i;
    int32_t rc;
    int32_t numFailed = 0;
    int32_t numPassed = 0;

    RBLI("Running %d test(s) ..", numTests);
    RBLI("Logging results to '" LOG_FILE "' ..");
    RBLI("-------------------------------------");

    pthread_t* testThreads = (pthread_t*) malloc(numTests * sizeof(pthread_t));

    for (i = 0; i < numTests; i++) {
        pthread_create(&testThreads[i], NULL, testRunner, (TestEntry*)&entries[i]);
    }

    for (i = 0; i < numTests; i++) {
        void* res = NULL;
        pthread_join(testThreads[i], &res);

        if ((intptr_t) res) {
            ++numFailed;
        } else {
            ++numPassed;
        }
    }

    free(testThreads);

    RBLI("-------------------------------------");
    RBLE("Tests failed: %d ( %.2f%% )", numFailed,
            ((float )numFailed / (float )numTests) * 100.0f);
    RBLI("Tests passed: %d ( %.2f%% )", numPassed,
            ((float )numPassed / (float )numTests) * 100.0f);

    return numFailed ? -1 : 0;
}

void* testRunner(void* arg) {
    const TestEntry* test = (const TestEntry*) arg;

    RBLI("Running test '%s'", test->name);

    int rc = test->fnc();

    if (rc) {
        RBLE("Test '%s' failed", test->name);
    } else {
        RBLI("Test '%s' passed", test->name);
    }

    return (void*) (intptr_t)rc;
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
