/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/ConcurrentRingBuffer.h>
#include <rb/Log.h>
#include <pthread.h>
#include <stdbool.h>

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "testConcurrency"

#define NUM_TEST_DATA ( 10 * 1024 )

static void* consumer(void* arg);

int testConcurrency() {
	if(!RB_CHECK_VERSION){
		RBLE("Invalid binary version");
		return -1;
	}

	int32_t rc;
	int32_t i;
	const int32_t kCAPACITY = 1024;

	CRingBufferHandle rb = CRingBuffer_new(kCAPACITY);
	if (rb == NULL) {
		RBLE("RingBuffer_new CRingBuffer_new");
		return -1;
	}

	pthread_t consumerThread;
	pthread_create(&consumerThread, NULL, consumer, (void*) rb);


	uint32_t bytesWritten = 0;
	while(bytesWritten != NUM_TEST_DATA){
		static const uint32_t kBFR_SIZE = 37;

		uint8_t data[kBFR_SIZE];

		uint32_t i;
		for(i=0; i<kBFR_SIZE; i++){
			data[i] = (bytesWritten + i ) % 0xFF;
		}

		uint32_t bytesLeft = NUM_TEST_DATA - bytesWritten;
		uint32_t toWrite = bytesLeft > kBFR_SIZE ? kBFR_SIZE : bytesLeft;

		rc = CRingBuffer_write(rb, data,
				toWrite, eWRITE_BLOCK_FULL);
		if(rc != (int32_t)toWrite){
			RBLE("CRingBuffer_write failed: %d");
			return -1;
		}

		bytesWritten += rc;
	}

	RBLI("Producer thread done");

	void* vrc = NULL;
	pthread_join(consumerThread, &vrc);
	if (vrc) {
		RBLE("Consumer thread failed");
		return -1;
	}

	rc = CRingBuffer_free(&rb);
	if (rc != RB_OK || rb != NULL) {
		RBLE("CRingBuffer_read failed");
	}

	return 0;
}

void* consumer(void* arg) {
	CRingBufferHandle rb = (CRingBufferHandle) arg;

	static const uint32_t kBUFFER_SIZE = 17;
	uint8_t bfr[kBUFFER_SIZE];
	uint32_t bytesRead = 0;

	while(bytesRead != NUM_TEST_DATA){
		uint32_t bytesLeft = NUM_TEST_DATA - bytesRead;
		uint32_t toRead = bytesLeft > kBUFFER_SIZE ? kBUFFER_SIZE : bytesLeft;

		int32_t rc = CRingBuffer_read(rb, bfr, toRead, eREAD_BLOCK_FULL);
		if(rc != (int32_t)toRead){
			RBLE("CRingBuffer_read failed: %d", rc);
			return (void*)-1;
		}

		int32_t i;
		for(i=0; i<rc; i++){
			if(bfr[i] != (bytesRead + i ) % 0xFF){
				RBLE("Invalid data read: %d != %d", bfr[i], bytesRead+i);
				return (void*)-1;
			}
		}

		bytesRead += rc;
	}

	RBLI("Consumer thread done");

	return (void*) 0;
}
