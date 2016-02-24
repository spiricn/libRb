/**
 * @file ConcurrentRingBuffer.c
 * @author Nikola Spiric <nikola.spiric@rt-rk.com>
 */

/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "ConcurrentRingBuffer.h"
#include "RingBuffer.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#define CONCURRENT_RING_BUFFER_MAGIC ( 0xC04C6B43 )

#define LOCK_ACQUIRE do{ pthread_mutex_lock(&rb->base->mutex); }while(0)

#define LOCK_RELEASE do{ pthread_mutex_unlock(&rb->base->mutex); }while(0)

#define READ_LOCK do{ pthread_mutex_lock(&rb->base->readMutex); }while(0)

#define READ_RELEASE do{ pthread_mutex_unlock(&rb->base->readMutex); }while(0)

#define WRITE_LOCK do{ pthread_mutex_lock(&rb->base->writeMutex); }while(0)

#define WRITE_RELEASE do{ pthread_mutex_unlock(&rb->base->writeMutex); }while(0)

#define VALID_HANDLE(handle) ( (handle) != NULL && (handle)->magic == CONCURRENT_RING_BUFFER_MAGIC )

/*******************************************************/
/*              Typedefs                               */
/*******************************************************/

typedef struct {
	pthread_mutex_t mutex;
	pthread_mutex_t readMutex;
	pthread_mutex_t writeMutex;
	pthread_cond_t readCV;
	pthread_cond_t writeCV;
	int enabled;
} CRingBufferBase;

typedef struct {
	uint32_t magic;
	CRingBufferBase* base;
	RingBufferHandle buffer;
	int sharedMemory;
	int owned;
} CRingBufferContext;

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static CRingBufferContext* CRingBufferPriv_getContext(CRingBufferHandle handle);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

CRingBufferHandle CRingBuffer_fromSharedMemory(void* memory, uint32_t size,
		int init) {
	if (size == 0) {
		return NULL;
	}

	CRingBufferContext* rb = (CRingBufferContext*) calloc(1,
			sizeof(CRingBufferContext));
	rb->base = (CRingBufferBase*) memory;

	rb->magic = CONCURRENT_RING_BUFFER_MAGIC;

	rb->buffer = RingBuffer_fromSharedMemory(
			((uint8_t*) memory) + sizeof(CRingBufferBase),
			size - sizeof(CRingBufferBase), init);

	if (init) {
		rb->base->enabled = 1;

		pthread_mutexattr_t mutexAttr;
		pthread_mutexattr_init(&mutexAttr);
		pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&rb->base->mutex, &mutexAttr);

		pthread_condattr_t condAttr;
		pthread_condattr_init(&condAttr);
		pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);

		pthread_cond_init(&rb->base->readCV, &condAttr);

		pthread_cond_init(&rb->base->writeCV, &condAttr);

		rb->owned = 1;
	} else {
		rb->owned = 0;
	}

	rb->sharedMemory = 1;

	return rb;
}

CRingBufferHandle CRingBuffer_new(uint32_t size) {
	if (size == 0) {
		return NULL;
	}

	CRingBufferContext* rb = (CRingBufferContext*) calloc(1,
			sizeof(CRingBufferContext));

	rb->base = (CRingBufferBase*) malloc(sizeof(CRingBufferBase));
	rb->magic = CONCURRENT_RING_BUFFER_MAGIC;

	pthread_mutex_init(&rb->base->mutex, NULL);
	pthread_mutex_init(&rb->base->readMutex, NULL);
	pthread_mutex_init(&rb->base->writeMutex, NULL);
	pthread_cond_init(&rb->base->readCV, NULL);
	pthread_cond_init(&rb->base->writeCV, NULL);

	rb->buffer = RingBuffer_new(size);
	rb->base->enabled = 1;
	rb->sharedMemory = 0;
	rb->owned = 1;

	return rb;
}

int32_t CRingBuffer_free(CRingBufferHandle* handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(*handle);
	if (rb == NULL) {
		return -1;
	}

	if (rb->owned) {
		pthread_mutex_destroy(&rb->base->mutex);
		pthread_mutex_destroy(&rb->base->writeMutex);
		pthread_mutex_destroy(&rb->base->readMutex);
		pthread_cond_destroy(&rb->base->readCV);
		pthread_cond_destroy(&rb->base->writeCV);
	}

	const int32_t res = RingBuffer_free(&rb->buffer);

	if (!rb->sharedMemory) {
		free(rb->base);
	}

	free(rb);
	*handle = NULL;

	return res;
}

int32_t CRingBuffer_read(CRingBufferHandle handle, uint8_t* data, uint32_t size,
		CRingBuffer_ReadMode mode) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	uint32_t bytesRead = 0;

	// Checkpoint
	if (!rb->base->enabled) {
		return 0;
	}

	READ_LOCK
	;

	LOCK_ACQUIRE
	;

	// Checkpoint
	if (!rb->base->enabled) {
		LOCK_RELEASE
		;
		READ_RELEASE
		;
		return 0;
	}

	if (mode == eREAD_BLOCK_FULL) {
		uint32_t bytesRemaining = size;
		uint32_t bytesUsed = 0;

		while (bytesRemaining) {
			// Wait until some data is available
			while ((bytesUsed = RingBuffer_getBytesUsed(rb->buffer)) == 0
					&& rb->base->enabled) {
				pthread_cond_wait(&rb->base->writeCV, &rb->base->mutex);
			}

			// Checkpoint
			if (!rb->base->enabled) {
				LOCK_RELEASE
				;
				READ_RELEASE
				;

				return size - bytesRemaining;
			}

			const int32_t toRead =
					bytesRemaining < bytesUsed ? bytesRemaining : bytesUsed;

			RingBuffer_read(rb->buffer, data + (size - bytesRemaining), toRead);

			bytesRemaining -= toRead;

			pthread_cond_broadcast(&rb->base->readCV);
		}

		bytesRead = size - bytesRemaining;
	} else {
		if (mode == eREAD_BLOCK_PARTIAL) {
			// Wait at least some of the data we requires is available
			while (RingBuffer_getBytesUsed(rb->buffer) == 0 && rb->base->enabled) {
				pthread_cond_wait(&rb->base->writeCV, &rb->base->mutex);
			}

			// Checkpoint
			if (!rb->base->enabled) {
				LOCK_RELEASE
				;
				READ_RELEASE
				;
				return 0;
			}

			uint32_t available = RingBuffer_getBytesUsed(rb->buffer);

			size = size > available ? available : size;
		} else if (mode == eREAD_BLOCK_NONE) {
			// Read whatever data is available at the moment (may be nothing)
			uint32_t available = RingBuffer_getBytesUsed(rb->buffer);

			size = size > available ? available : size;
		}

		if (size) {
			bytesRead = RingBuffer_read(rb->buffer, data, size);

			pthread_cond_broadcast(&rb->base->readCV);
		}
	}

	LOCK_RELEASE
	;
	READ_RELEASE
	;

	return bytesRead;
}

int32_t CRingBuffer_write(CRingBufferHandle handle, const uint8_t* data,
		uint32_t size, CRingBuffer_WriteMode mode) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	// Total bytes written
	uint32_t bytesWritten = 0;

	// Checkpoint
	if (!rb->base->enabled) {
		return 0;
	}

	WRITE_LOCK
	;

	LOCK_ACQUIRE
	;

	// Checkpoint
	if (!rb->base->enabled) {
		LOCK_RELEASE
		;
		WRITE_RELEASE
		;
		return 0;
	}

	if (mode == eWRITE_BLOCK_FULL) {
		uint32_t bytesRemaining = size;
		uint32_t bytesFree = 0;

		while (bytesRemaining) {
			// Wait until some space is free
			while ((bytesFree = RingBuffer_getBytesFree(rb->buffer)) == 0
					&& rb->base->enabled) {
				pthread_cond_wait(&rb->base->readCV, &rb->base->mutex);
			}

			// Checkpoint
			if (!rb->base->enabled) {
				LOCK_RELEASE
				;
				WRITE_RELEASE
				;

				return size - bytesRemaining;
			}

			const uint32_t toWrite =
					bytesRemaining < bytesFree ? bytesRemaining : bytesFree;

			RingBuffer_write(rb->buffer, data + (size - bytesRemaining),
					toWrite);

			bytesRemaining -= toWrite;

			pthread_cond_broadcast(&rb->base->writeCV);
		}

		bytesWritten = size - bytesRemaining;
	} else {
		if (mode == eWRITE_WRITE_SOME) {
			// Write as much data as we can without blocking
			uint32_t free = RingBuffer_getBytesFree(rb->buffer);

			size = size > free ? free : size;
		}

		if (size) {
			bytesWritten = RingBuffer_write(rb->buffer, data, size);

			pthread_cond_broadcast(&rb->base->writeCV);
		}
	}

	LOCK_RELEASE
	;
	WRITE_RELEASE
	;

	return bytesWritten;
}

int32_t CRingBuffer_getBytesUsed(CRingBufferHandle handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	LOCK_ACQUIRE
	;

	const int32_t res = RingBuffer_getBytesUsed(rb->buffer);

	LOCK_RELEASE
	;

	return res;
}

int32_t CRingBuffer_getBytesFree(CRingBufferHandle handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	LOCK_ACQUIRE
	;

	const int32_t res = RingBuffer_getBytesFree(rb->buffer);

	LOCK_RELEASE
	;

	return res;
}

int32_t CRingBuffer_getSize(CRingBufferHandle handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	LOCK_ACQUIRE
	;

	const int32_t res = RingBuffer_getSize(rb->buffer);

	LOCK_RELEASE
	;

	return res;
}

int32_t CRingBuffer_disable(CRingBufferHandle handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	LOCK_ACQUIRE
	;

	rb->base->enabled = 0;

	pthread_cond_broadcast(&rb->base->readCV);
	pthread_cond_broadcast(&rb->base->writeCV);

	LOCK_RELEASE
	;

	return 0;
}

int32_t CRingBuffer_enable(CRingBufferHandle handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	LOCK_ACQUIRE
	;

	rb->base->enabled = 1;

	LOCK_RELEASE
	;

	return 0;
}

int32_t CRingBuffer_isEnabled(CRingBufferHandle handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	LOCK_ACQUIRE
	;

	const int32_t res = rb->base->enabled;

	LOCK_RELEASE
	;

	return res;
}

int32_t CRingBuffer_clear(CRingBufferHandle handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	LOCK_ACQUIRE
	;

	RingBuffer_clear(rb->buffer);

	pthread_cond_broadcast(&rb->base->readCV);

	LOCK_RELEASE
	;

	return 0;
}

int32_t CRingBuffer_isFull(CRingBufferHandle handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	LOCK_ACQUIRE
	;

	int32_t res = RingBuffer_isFull(rb->buffer);

	LOCK_RELEASE
	;

	return res;
}

int32_t CRingBuffer_isEmpty(CRingBufferHandle handle) {
	CRingBufferContext* rb = CRingBufferPriv_getContext(handle);
	if (rb == NULL) {
		return -1;
	}

	LOCK_ACQUIRE
	;

	int32_t res = RingBuffer_isEmpty(rb->buffer);

	LOCK_RELEASE
	;

	return res;
}

CRingBufferContext* CRingBufferPriv_getContext(CRingBufferHandle handle) {
	if (handle == NULL) {
		return NULL;
	}

	CRingBufferContext* rb = (CRingBufferContext*) handle;
	if (rb->magic != CONCURRENT_RING_BUFFER_MAGIC) {
		return NULL;
	}

	return rb;
}
