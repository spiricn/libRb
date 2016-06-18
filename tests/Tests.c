/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <Log.h>


/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestBuffer"
#define ADD_TEST(x) {x, #x},
#define NUM_TESTS ( 5 )

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

extern int testBuffer();
extern int testCBuffer();
extern int testConcurrency();
extern int testArray();
extern int testMessageBox();

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef int (*testFnc)();;

typedef struct {
	testFnc fnc;
	char* name;
} TestEntry;

/********************************************************/
/*                 Local Module Variables (MODULE)      */
/********************************************************/

TestEntry gTests[NUM_TESTS] = {
		ADD_TEST(testBuffer)
		ADD_TEST(testCBuffer)
		ADD_TEST(testConcurrency)
		ADD_TEST(testArray)
		ADD_TEST(testMessageBox)
};

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int main() {
	int32_t i;
	int32_t numFailed = 0;
	int32_t numPassed = 0;

	RBLI("Running %d test(s) ..", NUM_TESTS);

	for(i=0; i<NUM_TESTS; i++){
		TestEntry* test = &gTests[i];

		RBLI("-------------------------------------");
		RBLI("Running test '%s' (%d/%d)", test->name, i+1, NUM_TESTS);

		int32_t rc = test->fnc();

		if(rc){
			RBLE("Test failed: %d", rc);
			++numFailed;
		}
		else{
			RBLI("Test OK");
			++numPassed;
		}
	}

	RBLI("-------------------------------------");
	RBLE("Tests failed: %d ( %.2f%% )", numFailed, ((float)numFailed/(float)NUM_TESTS)*100.0f);
	RBLI("Tests passed: %d ( %.2f%% )", numPassed, ((float)numPassed/(float)NUM_TESTS)*100.0f);

	return numFailed ? -1 : 0;
}
