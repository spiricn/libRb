###############################################################################

Building the library & test binaries on Linux via CMake:

1) cd build/cmake

2) cmake -G "Unix Makefiles"

3) make all

Outputs are:

	a) libRingBuffer library
		build/cmake/lib/libRingBuffer.so
		
	b) Test binary
		build/cmake/bin/libRingBuffer_tests
		
###############################################################################

Building the library & test binaries on Android:

1) cd build/android

2) mm -B

###############################################################################		