/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/MessageBox.h>
#include <rb/Log.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestMessageBox"

#define NUM_MESSAGES ( 32 )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
	int32_t test;
} Message;

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testMessageBox() {
	if(!RB_CHECK_VERSION){
		RBLE("Invalid binary version");
		return -1;
	}

	int32_t rc;

	// Create message box
	Rb_MessageBoxHandle mb = Rb_MessageBox_new(sizeof(Message), NUM_MESSAGES);
	if(!mb){
		RBLE("Rb_MessageBox_new failed");
		return -1;
	}

	// Check capacity
	if(Rb_MessageBox_getCapacity(mb) != NUM_MESSAGES){
		RBLE("Rb_MessageBox_getCapacity failed");
		return -1;
	}

	// Check if it's empty
	if(Rb_MessageBox_getNumMessages(mb)){
		RBLE("Rb_MessageBox_getNumMessages failed");
		return -1;
	}

	Message msgIn = { 42 };
	Message msgOut;

	// Write message
	rc = Rb_MessageBox_write(mb, &msgIn);
	if(rc != RB_OK){
		RBLE("Rb_MessageBox_write failed");
		return -1;
	}

	if(Rb_MessageBox_getNumMessages(mb) != 1){
		RBLE("Rb_MessageBox_getNumMessages failed");
		return -1;
	}

	// Read message
	rc = Rb_MessageBox_read(mb, &msgOut);
	if(rc != RB_OK){
		RBLE("Rb_MessageBox_read failed");
		return -1;
	}

	// Check data
	if(memcmp(&msgIn, &msgOut, sizeof(Message))){
		RBLE("Invalid data");
		return -1;
	}

	// Destroy message box
	rc = Rb_MessageBox_free(&mb);
	if(rc != RB_OK && mb){
		RBLE("Rb_MessageBox_free failed");
		return -1;
	}

	return 0;
}
