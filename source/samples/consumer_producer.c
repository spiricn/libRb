#include "ConcurrentRingBuffer.h"

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>

#define BUFFER_SIZE ( 50 * 1024 * 1024 )

#define PRODUCER_BFR_SIZE ( 300 )

#define CONSUMER_BFR_SIZE ( 600 )

void* consumerThread(void* arg);

void* producerThread(void* arg);

int main(){
	void* producerRes = NULL;
	void* consumerRes = NULL;
	pthread_t producer, consumer;

	// Seed the RNG
	srand(time(NULL));

	// Create a concurrent ring buffer
	CRingBuffer bfr = CRingBuffer_new( BUFFER_SIZE );

	// Start the producer thread
	pthread_create(&producer, NULL, producerThread, bfr);

	// Start the consumer thread
	pthread_create(&consumer, NULL, consumerThread, bfr);


	// Wait untill they're finished
	pthread_join(producer, &producerRes);
	pthread_join(consumer, &consumerRes);

	// Relase the buffer
	CRingBuffer_free(&bfr);

	if( producerRes == NULL && consumerRes == NULL ){

		printf("#################\n");
		printf("Test OK\n");
		printf("#################\n");

		return 0;
	}
	else{

		printf("#################\n");
		printf("Test failed\n");
		printf("#################\n");

		return 1;
	}
}


void* consumerThread(void* arg){
	CRingBuffer buffer = (CRingBuffer)arg;
	uint8_t data[CONSUMER_BFR_SIZE];
	int totalRead = 0;
	uint8_t counter = 0;
	void* error = NULL;
	int i;

	while(totalRead < BUFFER_SIZE){
		// Read data (block until there's at least some data to read)
		int read = CRingBuffer_read(buffer, data, rand() % CONSUMER_BFR_SIZE, eREAD_BLOCK_PARTIAL);


		for(i=0; i<read; i++){
			// Verify data correctness
			if(data[i] != counter++){
				// Corrupt data
				return (void*)1;
			}
		}

		// Repeat until we consumed BUFFER_SIZE bytes
		totalRead += read;
	}

	printf("Consumer thread finished\n");

	return error;
}

void* producerThread(void* arg){
	CRingBuffer buffer = (CRingBuffer)arg;
	uint8_t data[PRODUCER_BFR_SIZE];
	int totalWritten = 0;
	int i = 0;

	// Used for veryfing data
	uint8_t counter = 0;

	while(totalWritten < BUFFER_SIZE){

		// Write a random number of bytes
		int toWrite = rand() % PRODUCER_BFR_SIZE;

		// Set data buffer
		for(i=0; i<toWrite; i++){
			data[i] = counter++;
		}

		// Write data (block until everything has been written)
		int written = CRingBuffer_write(buffer, data, toWrite, eWRITE_BLOCK_FULL);

		// Repeat untill we write BUFFER_SIZE bytes
		totalWritten += written;
	}

	printf("Producer thread finished\n");

	return NULL;
}
