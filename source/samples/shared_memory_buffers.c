#include "RingBuffer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define BUFFER_SIZE ( 1024 )

int main(){
	int i;
	uint8_t outBfr[BUFFER_SIZE];
	uint8_t inBfr[BUFFER_SIZE];

	// Get some memory (e.g. shared memory between processes)
	void* sharedMemory = malloc(BUFFER_SIZE);

	// Create the first buffer from memory and initialize it
	RingBuffer buffer1 = RingBuffer_fromSharedMemory(sharedMemory, BUFFER_SIZE,
			/* initialize buffer */ 1);
	assert( buffer1 );

	// Create the second buffer (do not initialize this time)
	RingBuffer buffer2 = RingBuffer_fromSharedMemory(sharedMemory, BUFFER_SIZE,
				/* do NOT initialize the buffer (it's already been done)*/ 0);
	assert( buffer2 );

	// Write some data to the first buffer
	int bytesFree = RingBuffer_getBytesFree(buffer1);
	memset(inBfr, 0x00, bytesFree);
	for(i=0; i<bytesFree; i++){
		inBfr[i] = i % 0xFF;
	}
	assert( RingBuffer_write(buffer1, inBfr, bytesFree) == bytesFree );

	// Read previously written data from the second buffer
	int bytesUsed = RingBuffer_getBytesUsed(buffer2);
	memset(outBfr, 0x00, bytesUsed);
	assert( RingBuffer_read(buffer2, outBfr, bytesUsed) == bytesUsed );

	// Verify data correctness
	for(i=0; i<bytesUsed; i++){
		assert( outBfr[i] == inBfr[i] );
	}

	// Release buffers
	assert( RingBuffer_free(&buffer1) == 0 );

	assert( RingBuffer_free(&buffer2) == 0 );

	// Release shared memory
	free(sharedMemory);

	printf("#################\n");
	printf("Test OK\n");
	printf("#################\n");

	return 0;
}
