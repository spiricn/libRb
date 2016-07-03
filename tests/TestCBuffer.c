/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/ConcurrentRingBuffer.h>
#include <rb/Log.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define TIMEOUT_MS ( 500 )

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestCBuffer"

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testCBuffer() {
	if(!RB_CHECK_VERSION){
		RBLE("Invalid binary version");
		return -1;
	}

	int32_t rc;
	int32_t i;
	const int32_t kCAPACITY = 10;

	uint8_t testData[kCAPACITY];
	uint8_t testOutData[kCAPACITY];

	// Initialize some data
	for(i=0; i<kCAPACITY; i++){
		testData[i] = i;
	}

	Rb_CRingBufferHandle rb = Rb_CRingBuffer_new(kCAPACITY);
	if (rb == NULL) {
		RBLE("Rb_RingBuffer_new Rb_CRingBuffer_new");
		return -1;
	}

	if (Rb_CRingBuffer_getCapacity(rb) != kCAPACITY) {
		RBLE("Rb_CRingBuffer_getCapacity failed");
		return -1;
	}

	if (Rb_CRingBuffer_getBytesFree(rb) != kCAPACITY) {
		RBLE("Rb_CRingBuffer_getBytesFree failed");
		return -1;
	}

	if (Rb_CRingBuffer_getBytesUsed(rb) != 0) {
		RBLE("Rb_CRingBuffer_getBytesUsed failed");
		return -1;
	}

	if(!Rb_CRingBuffer_isEmpty(rb) || Rb_CRingBuffer_isFull(rb)){
		RBLE("Rb_CRingBuffer_isEmpty or Rb_CRingBuffer_isFull failed");
		return -1;
	}

	// Timed read
	rc = Rb_CRingBuffer_readTimed(rb, testOutData, kCAPACITY, eRB_READ_BLOCK_FULL, TIMEOUT_MS);
	if(rc != RB_TIMEOUT){
		RBLE("Rb_CRingBuffer_readTimed");
		return -1;
	}

	// Write
	rc = Rb_CRingBuffer_write(rb, testData, kCAPACITY, eRB_WRITE_BLOCK_FULL);
	if(rc != kCAPACITY){
		RBLE("Rb_CRingBuffer_write failed");
		return -1;
	}

	// Make sure buffer is full
	if(Rb_CRingBuffer_isEmpty(rb) || !Rb_CRingBuffer_isFull(rb)){
		RBLE("Rb_CRingBuffer_isEmpty or Rb_CRingBuffer_isFull failed");
		return -1;
	}

	if(Rb_CRingBuffer_getBytesFree(rb)){
		RBLE("Rb_CRingBuffer_getBytesFree failed");
		return -1;
	}

	if(Rb_CRingBuffer_getBytesUsed(rb) != kCAPACITY){
		RBLE("Rb_CRingBuffer_getBytesUsed failed");
		return -1;
	}

	// Write timeout test
	rc = Rb_CRingBuffer_writeTimed(rb, testData, kCAPACITY, eRB_WRITE_BLOCK_FULL, TIMEOUT_MS);
	if(rc != RB_TIMEOUT){
		RBLE("Rb_CRingBuffer_writeTimed failed");
		return -1;
	}

	rc = Rb_CRingBuffer_read(rb, testOutData, kCAPACITY, eRB_READ_BLOCK_FULL);
	if(rc != kCAPACITY){
		RBLE("Rb_CRingBuffer_read failed");
		return -1;
	}

	// Make sure buffer is empty
	if(!Rb_CRingBuffer_isEmpty(rb) || Rb_CRingBuffer_isFull(rb)){
		RBLE("Rb_CRingBuffer_isEmpty or Rb_CRingBuffer_isFull failed");
		return -1;
	}

	// Test read data
	if(memcmp(testOutData, testData, kCAPACITY)){
		RBLE("Invalid data read");
		return -1;
	}

	rc = Rb_CRingBuffer_free(&rb);
	if (rc != RB_OK || rb != NULL) {
		RBLE("Rb_CRingBuffer_read failed");
	}

	return 0;
}
