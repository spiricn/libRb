CC=gcc
PWD=$(shell pwd)
OUT=$(PWD)/out
LIBROOT=$(PWD)/../..
SRCDIR=$(LIBROOT)/source
INCDIR=$(LIBROOT)/include
TARGET_LIB = libRingBuffer.so
SOURCES=\
	RingBuffer.c \
	Stopwatch.c \
	ConcurrentRingBuffer.c \
	MessageBox.c \
	Log.c \

OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-fPIC \
		-I$(INCDIR)
LDFLAGS=-shared


$(SOURCES):
	$(CC) $(CFLAGS) -c $(SRCDIR)/$@ 

$(TARGET_LIB): init $(SOURCES)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(OUT)/$(TARGET_LIB)

init:
	mkdir -p $(OUT)

all: $(TARGET_LIB)

clean:
	rm -f *.o
	rm -rf $(OUT)
