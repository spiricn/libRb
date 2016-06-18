cmake_minimum_required(VERSION 2.8)

set(CPACK_PACKAGE_NAME "libRingBuffer")
set(CPACK_PACKAGE_VENDOR "libRingBuffer-Vendor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "libRingBuffer library")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

###############################################################################

project("libHTTPServer")


set(SOURCE_DIR "../source")

set(INCLUDE_DIR "../include")


set(SOURCES
	${SOURCE_DIR}/ConcurrentRingBuffer.c
	${SOURCE_DIR}/Log.c
	${SOURCE_DIR}/MessageBox.c
	${SOURCE_DIR}/RingBuffer.c
	${SOURCE_DIR}/Stopwatch.c
)

set(HEADERS
	${INCLUDE_DIR}/Common.h
	${INCLUDE_DIR}/ConcurrentRingBuffer.h
	${INCLUDE_DIR}/Log.h
	${INCLUDE_DIR}/MessageBox.h
	${INCLUDE_DIR}/RingBuffer.h
)

include_directories(${INCLUDE_DIR})


# With -fPIC
IF(UNIX AND NOT WIN32)
  FIND_PROGRAM(CMAKE_UNAME uname /bin /usr/bin /usr/local/bin )
  IF(CMAKE_UNAME)
    EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR)
    SET(CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} CACHE INTERNAL
"processor type (i386 and x86_64)")
    IF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
      ADD_DEFINITIONS(-fPIC)
    ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  ENDIF(CMAKE_UNAME)
ENDIF(UNIX AND NOT WIN32)

add_library(
	RingBuffer SHARED
	${SOURCES}
	${HEADERS}
)


add_executable(test_buffers "../tests/TestBuffer.c")

TARGET_LINK_LIBRARIES( test_buffers
	RingBuffer
	pthread
)
