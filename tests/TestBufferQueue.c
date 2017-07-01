/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/BufferQueue.h>
#include <rb/Log.h>
#include <rb/Common.h>

#include <stdio.h>
#include <pthread.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestBufferQueue"

#define NUM_BUFFERS ( 32 )
#define BUFFER_SIZE ( 1024 * 4)
#define NUM_ITERATIONS ( 10 )

#define INPUT_BYTE(i) (i%0xFF)
#define OUTPUT_BYTE(i) (0xFF - (i%0xFF))

/*******************************************************/
/*              Functions Declarations                 */
/*******************************************************/

static void* inputThread(void* arg);

static void* processorThread(void* arg);

static void* outputThread(void* arg);

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testBufferQueue() {
    int32_t rc;
    int32_t i;

    Rb_BufferQueueHandle bq;

    bq = Rb_BufferQueue_new(NUM_BUFFERS, BUFFER_SIZE);
    if (bq == NULL) {
        RBLE("Rb_BufferQueue_new failed");
        return -1;
    }

    for (i = 0; i < NUM_BUFFERS; i++) {
        rc = Rb_BufferQueue_containsBuffer(bq, i);
        if (rc < 0) {
            RBLE("Rb_BufferQueue_containsBuffer failed");
            return -1;
        }

        if (rc != RB_TRUE) {
            RBLE("Buffer not added");
            return -1;
        }
    }

    // Reader thread will finish first
    pthread_t inputTid;
    pthread_create(&inputTid, NULL, inputThread, (void*) bq);

    pthread_t processorTid;
    pthread_create(&processorTid, NULL, processorThread, (void*) bq);

    void* vrc = NULL;

    pthread_join(inputTid, &vrc);
    if ((intptr_t) vrc != 0) {
        RBLE("Input thread failed");
        return -1;
    }

    pthread_join(processorTid, &vrc);
    if ((intptr_t) vrc != 0) {
        RBLE("Processor thread failed");
        return -1;
    }

    rc = Rb_BufferQueue_free(&bq);
    if (rc != RB_OK || bq) {
        RBLE("Rb_BufferQueue_new failed");
        return -1;
    }

    return 0;
}

void* inputThread(void* arg) {
    Rb_BufferQueueHandle bq = (Rb_BufferQueueHandle) arg;

    int32_t counter;
    int32_t index;
    int32_t rc;
    int32_t i;

    for (i = 0; i < NUM_ITERATIONS; i++) {
        // Get a free buffer
        rc = Rb_BufferQueue_dequeueFreeBuffer(bq, &index, RB_WAIT_INFINITE);
        if (rc != RB_OK) {
            RBLE("Rb_BufferQueue_dequeueInputBuffer failed");
            return (void*) -1;
        }

        void* data = NULL;
        int32_t size = 0;

        rc = Rb_BufferQueue_getBuffer(bq, index, &data, &size);
        if (rc != RB_OK) {
            RBLE("Rb_BufferQueue_getBuffer failed");
            return (void*) -1;
        }

        // Fill it with input data
        memset(data, INPUT_BYTE(i), size);

        // Queue buffer for processing
        rc = Rb_BufferQueue_queueInputBuffer(bq, index);
        if (rc != RB_OK) {
            RBLE("Rb_BufferQueue_queueInputBuffer failed");
            return (void*) -1;
        }
    }

    return NULL;
}

void* processorThread(void* arg) {
    Rb_BufferQueueHandle bq = (Rb_BufferQueueHandle) arg;

    int32_t index;
    int32_t rc;
    int32_t i;
    int32_t j;

    for (i = 0; i < NUM_ITERATIONS; i++) {
        // Dequeue input buffer submited for processing by the input thread
        rc = Rb_BufferQueue_dequeueInputBuffer(bq, &index, RB_WAIT_INFINITE);
        if (rc != RB_OK) {
            RBLE("Rb_BufferQueue_dequeueInputBuffer failed");
            return (void*) -1;
        }

        void* data = NULL;
        int32_t size = 0;

        rc = Rb_BufferQueue_getBuffer(bq, index, &data, &size);
        if (rc != RB_OK) {
            RBLE("Rb_BufferQueue_getBuffer failed");
            return (void*) -1;
        }

        if (!data || size != BUFFER_SIZE) {
            RBLE("Invalid buffer data/size");
            return (void*) -1;
        }

        // Verify input data
        for (j = 0; j < size; j++) {
            uint8_t inByte = ((uint8_t*) data)[j];
            uint8_t expectedInput = INPUT_BYTE(i);

            if (inByte != expectedInput) {
                RBLE("Invalid data: index=%d, %x != %x", j, inByte, i);
                return (void*) -1;
            }
        }

        // Process input data
        memset(data, OUTPUT_BYTE(i), size);

        // Produce output buffer
        rc = Rb_BufferQueue_queueFreeBuffer(bq, index);
        if (rc != RB_OK) {
            RBLE("Rb_BufferQueue_queueInputBuffer failed");
            return (void*) -1;
        }
    }

    return NULL;
}
