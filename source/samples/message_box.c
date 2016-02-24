#include "MessageBox.h"

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

void* consumerThread(void* arg);

void* producerThread(void* arg);

#define NUM_MESSAGES ( 3 )

#define PAYLOAD_SIZE ( 64 )

typedef struct Message_t {
    int messageNum;
    char payload[PAYLOAD_SIZE];
} Message;

int main() {
    void* producerRes = NULL;
    void* consumerRes = NULL;
    pthread_t producer, consumer;

    // Seed the RNG
    srand(time(NULL));

    // Create a concurrent ring buffer
    MessageBox mb = MessageBox_new(sizeof(Message), 10);

    // Start the producer thread
    pthread_create(&producer, NULL, producerThread, mb);

    // Start the consumer thread
    pthread_create(&consumer, NULL, consumerThread, mb);

    // Wait untill they're finished
    pthread_join(producer, &producerRes);
    pthread_join(consumer, &consumerRes);

    // Relase the buffer
    MessageBox_free(&mb);

    printf("Done\n");

    return 0;
}

void* consumerThread(void* arg) {
    MessageBox mb = (MessageBox)arg;

    printf("Consumer started\n");

    int messagesRecieved = 0;

    while(messagesRecieved < NUM_MESSAGES) {
        // Read message
        Message msg;

        assert(MessageBox_read(mb, &msg) == 0 && "MessageBox_read failed");

        assert(msg.messageNum == messagesRecieved && "Corrupt data read");

        int i;
        for(i = 0; i < PAYLOAD_SIZE; i++) {
            assert(msg.payload[i] == i % 0xFF && "Corrupt data read");
        }

        ++messagesRecieved;
    }

    printf("Consumer finished OK\n");

    return NULL;
}

void* producerThread(void* arg) {
    MessageBox mb = (MessageBox)arg;

    printf("Producer started\n");

    int messagesSent = 0;

    while(messagesSent < NUM_MESSAGES) {
        // Generate message
        Message msg;

        msg.messageNum = messagesSent++;

        int i;
        for(i = 0; i < PAYLOAD_SIZE; i++) {
            msg.payload[i] = i % 0xFF;
        }

        // Write message
        assert(MessageBox_write(mb, &msg) == 0 && "MessageBox_write failed");

        usleep(1e6 / 10);
    }

    printf("Producer finished OK\n");

    return NULL;
}
