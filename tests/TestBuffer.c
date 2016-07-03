/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/RingBuffer.h>
#include <rb/Log.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestBuffer"

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testBuffer() {
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

	Rb_RingBufferHandle rb = Rb_RingBuffer_new(kCAPACITY);
	if (rb == NULL) {
		RBLE("Rb_RingBuffer_new failed");
		return -1;
	}

	if (Rb_RingBuffer_getCapacity(rb) != kCAPACITY) {
		RBLE("Rb_RingBuffer_getCapacity failed");
		return -1;
	}

	if (Rb_RingBuffer_getBytesFree(rb) != kCAPACITY) {
		RBLE("Rb_RingBuffer_getBytesFree failed");
		return -1;
	}

	if (Rb_RingBuffer_getBytesUsed(rb) != 0) {
		RBLE("Rb_RingBuffer_getBytesUsed failed");
		return -1;
	}

	if(!Rb_RingBuffer_isEmpty(rb) || Rb_RingBuffer_isFull(rb)){
		RBLE("Rb_RingBuffer_isEmpty or Rb_RingBuffer_isFull failed");
		return -1;
	}

	// Write some data
	rc = Rb_RingBuffer_write(rb, testData, kCAPACITY);
	if(rc != kCAPACITY){
		RBLE("Rb_RingBuffer_write failed");
		return -1;
	}

	// Make sure buffer is full
	if(Rb_RingBuffer_isEmpty(rb) || !Rb_RingBuffer_isFull(rb)){
		RBLE("Rb_RingBuffer_isEmpty or Rb_RingBuffer_isFull failed");
		return -1;
	}

	if(Rb_RingBuffer_getBytesFree(rb)){
		RBLE("Rb_RingBuffer_getBytesFree failed");
		return -1;
	}

	if(Rb_RingBuffer_getBytesUsed(rb) != kCAPACITY){
		RBLE("Rb_RingBuffer_getBytesUsed failed");
		return -1;
	}

	// Read previously written data
	rc = Rb_RingBuffer_read(rb, testOutData, kCAPACITY);
	if(rc != kCAPACITY){
		RBLE("Rb_RingBuffer_read failed");
		return -1;
	}

	// Test read data
	if(memcmp(testOutData, testData, kCAPACITY)){
		RBLE("Invalid data read");
		return -1;
	}

	// Make sure buffer is empty
	if(!Rb_RingBuffer_isEmpty(rb) || Rb_RingBuffer_isFull(rb)){
		RBLE("Rb_RingBuffer_isEmpty or Rb_RingBuffer_isFull failed");
		return -1;
	}

	// Write some data (fill it up)
	rc = Rb_RingBuffer_write(rb, testData, kCAPACITY);
	if(rc != kCAPACITY){
		RBLE("Rb_RingBuffer_write failed");
		return -1;
	}

	const int32_t kNEW_CAPACITY = kCAPACITY*2;

	rc = Rb_RingBuffer_resize(rb, kNEW_CAPACITY);
	if(rc != RB_OK){
		RBLE("Rb_RingBuffer_resize failed");
		return -1;
	}

	if(Rb_RingBuffer_getCapacity(rb) != kNEW_CAPACITY){
		RBLE("Rb_RingBuffer_getCapacity failed");
		return -1;
	}

	uint8_t largeOutData[kNEW_CAPACITY];
	uint8_t largeTestData[kNEW_CAPACITY];
	for(i=0; i<kNEW_CAPACITY; i++){
		largeTestData[i] = i % 0xFF;
	}

	rc = Rb_RingBuffer_write(rb, largeTestData, kNEW_CAPACITY);
	if(rc != kNEW_CAPACITY){
		RBLE("Rb_RingBuffer_write failed");
		return -1;
	}

	rc = Rb_RingBuffer_read(rb, largeOutData, kNEW_CAPACITY);
	if(rc != kNEW_CAPACITY){
		RBLE("Rb_RingBuffer_read failed");
		return -1;
	}

	for(i=0; i<kNEW_CAPACITY; i++){
		if(largeOutData[i] != i % 0xFF){
			RBLE("Invalid data");
			return -1;
		}
	}

	rc = Rb_RingBuffer_free(&rb);
	if (rc != RB_OK || rb != NULL) {
		RBLE("Rb_RingBuffer_free failed");
	}

	return 0;
}
