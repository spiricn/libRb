#include "ConcurrentRingBuffer.h"

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>

#define BUFFER_SIZE ( 3 )

#define DATA_SIZE ( 5 * 1024 )

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

	uint8_t data[DATA_SIZE];

	printf("Reading %d bytes..\n", DATA_SIZE);

	const int32_t read  = CRingBuffer_read(buffer, data, DATA_SIZE, eREAD_BLOCK_FULL);

	printf("%d bytes read\n", read);

	if(read == DATA_SIZE){
		int i;
		for(i=0; i<DATA_SIZE; i++){
			if(data[i] != (i % 0xFF) ){
				// Corrupt data
				printf("Data corrupted @ %d\n", i);

				return (void*)1;
			}
		}

		return NULL;
	}
	else{
		return (void*)1;
	}
}

void* producerThread(void* arg){
	CRingBuffer buffer = (CRingBuffer)arg;

	uint8_t data[DATA_SIZE];

	int i = 0;
	for(i=0; i<DATA_SIZE; i++){
		data[i] = i % 0xFF;
	}

	printf("Writing %d bytes..\n", DATA_SIZE);

	int32_t written = CRingBuffer_write(buffer, data, DATA_SIZE, eWRITE_BLOCK_FULL);

	printf("%d bytes written\n", written);

	return (void*) ( written == DATA_SIZE ? 0 : 1);
}
