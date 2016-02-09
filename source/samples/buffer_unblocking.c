#include "ConcurrentRingBuffer.h"

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>

void* consumerThread(void* arg);


int main(){
	pthread_t consumer;
	CRingBuffer buffer = CRingBuffer_new(1024);

	// Start the producer thread
	pthread_create(&consumer, NULL, consumerThread, buffer);


	printf("Press any key to unblock the consumer thread\n");
	getchar();

	// Unblock consumer
	CRingBuffer_disable(buffer);

	// Wait for the thread to finish
	pthread_join(consumer, NULL);

	CRingBuffer_free(&buffer);

	printf("#################\n");
	printf("Test OK\n");
	printf("#################\n");

	return 0;
}

void* consumerThread(void* arg){
	CRingBuffer buffer = (CRingBuffer)arg;

	uint8_t dummy;

	// Block untill disable is called
	printf("Blocking consumer...\n");

	CRingBuffer_read(buffer, &dummy, 1, eREAD_BLOCK_FULL);

	printf("Consumer unblocked\n");

	return NULL;
}

