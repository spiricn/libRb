/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <ConcurrentRingBuffer.h>
#include <Log.h>

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
	int32_t rc;
	int32_t i;
	const int32_t kCAPACITY = 10;

	uint8_t testData[kCAPACITY];
	uint8_t testOutData[kCAPACITY];

	// Initialize some data
	for(i=0; i<kCAPACITY; i++){
		testData[i] = i;
	}

	CRingBufferHandle rb = CRingBuffer_new(kCAPACITY);
	if (rb == NULL) {
		RBLE("RingBuffer_new CRingBuffer_new");
		return -1;
	}

	if (CRingBuffer_getCapacity(rb) != kCAPACITY) {
		RBLE("CRingBuffer_getCapacity failed");
		return -1;
	}

	if (CRingBuffer_getBytesFree(rb) != kCAPACITY) {
		RBLE("CRingBuffer_getBytesFree failed");
		return -1;
	}

	if (CRingBuffer_getBytesUsed(rb) != 0) {
		RBLE("CRingBuffer_getBytesUsed failed");
		return -1;
	}

	if(!CRingBuffer_isEmpty(rb) || CRingBuffer_isFull(rb)){
		RBLE("CRingBuffer_isEmpty or CRingBuffer_isFull failed");
		return -1;
	}

	// Timed read
	rc = CRingBuffer_readTimed(rb, testOutData, kCAPACITY, eREAD_BLOCK_FULL, TIMEOUT_MS);
	if(rc != RB_TIMEOUT){
		RBLE("CRingBuffer_readTimed");
		return -1;
	}

	// Write
	rc = CRingBuffer_write(rb, testData, kCAPACITY, eWRITE_BLOCK_FULL);
	if(rc != kCAPACITY){
		RBLE("CRingBuffer_write failed");
		return -1;
	}

	// Make sure buffer is full
	if(CRingBuffer_isEmpty(rb) || !CRingBuffer_isFull(rb)){
		RBLE("CRingBuffer_isEmpty or CRingBuffer_isFull failed");
		return -1;
	}

	if(CRingBuffer_getBytesFree(rb)){
		RBLE("CRingBuffer_getBytesFree failed");
		return -1;
	}

	if(CRingBuffer_getBytesUsed(rb) != kCAPACITY){
		RBLE("CRingBuffer_getBytesUsed failed");
		return -1;
	}

	// Write timeout test
	rc = CRingBuffer_writeTimed(rb, testData, kCAPACITY, eWRITE_BLOCK_FULL, TIMEOUT_MS);
	if(rc != RB_TIMEOUT){
		RBLE("CRingBuffer_writeTimed failed");
		return -1;
	}

	rc = CRingBuffer_read(rb, testOutData, kCAPACITY, eREAD_BLOCK_FULL);
	if(rc != kCAPACITY){
		RBLE("CRingBuffer_read failed");
		return -1;
	}

	// Make sure buffer is empty
	if(!CRingBuffer_isEmpty(rb) || CRingBuffer_isFull(rb)){
		RBLE("CRingBuffer_isEmpty or CRingBuffer_isFull failed");
		return -1;
	}

	// Test read data
	if(memcmp(testOutData, testData, kCAPACITY)){
		RBLE("Invalid data read");
		return -1;
	}

	rc = CRingBuffer_free(&rb);
	if (rc != RB_OK || rb != NULL) {
		RBLE("CRingBuffer_read failed");
	}

	return 0;
}
