cmake_minimum_required(VERSION 2.8)

set(CPACK_PACKAGE_NAME "libRingBuffer")
set(CPACK_PACKAGE_VENDOR "libRingBuffer-Vendor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "libRingBuffer library")
set(CPACK_PACKAGE_VERSION "1.1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

###############################################################################

project("libRingBuffer")


set(LIB_ROOT "../../")

set(SOURCE_DIR "${LIB_ROOT}/source")

set(INCLUDE_DIR "${LIB_ROOT}/include")


set(SOURCES
	${SOURCE_DIR}/ConcurrentRingBuffer.c
	${SOURCE_DIR}/Log.c
	${SOURCE_DIR}/LogPriv.c
	${SOURCE_DIR}/MessageBox.c
	${SOURCE_DIR}/RingBuffer.c
	${SOURCE_DIR}/Stopwatch.c
	${SOURCE_DIR}/Array.c
	${SOURCE_DIR}/Common.c
	${SOURCE_DIR}/List.c
	${SOURCE_DIR}/Prefs.c
	${SOURCE_DIR}/PrefsBackend.c
	${SOURCE_DIR}/Utils.c
	${SOURCE_DIR}/IOStream.c
	${SOURCE_DIR}/FileStream.c
	${SOURCE_DIR}/Timer.c
	${SOURCE_DIR}/ErrorPriv.c
)

set(HEADERS
	${INCLUDE_DIR}/rb/Common.h
	${INCLUDE_DIR}/rb/ConcurrentRingBuffer.h
	${INCLUDE_DIR}/rb/Log.h
	${INCLUDE_DIR}/rb/MessageBox.h
	${INCLUDE_DIR}/rb/RingBuffer.h
	${INCLUDE_DIR}/rb/Array.h
	${INCLUDE_DIR}/rb/List.h
	${INCLUDE_DIR}/rb/Prefs.h
	${INCLUDE_DIR}/rb/Utils.h
	${INCLUDE_DIR}/rb/IOStream.h
	${INCLUDE_DIR}/rb/FileStream.h
	${INCLUDE_DIR}/rb/Timer.h
	${INCLUDE_DIR}/rb/Stopwatch.h
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

find_package (Threads)

add_library(
	RingBuffer SHARED
	${SOURCES}
	${HEADERS}
)

add_library(
	RingBufferStatic STATIC
	${SOURCES}
	${HEADERS}
)

target_link_libraries (RingBuffer ${CMAKE_THREAD_LIBS_INIT})

set(TEST_DIR "${LIB_ROOT}/tests")

set(TEST_SOURCES
	${TEST_DIR}/TestBuffer.c
	${TEST_DIR}/TestCBuffer.c
	${TEST_DIR}/TestConcurrency.c
	${TEST_DIR}/TestMessageBox.c
	${TEST_DIR}/TestArray.c
	${TEST_DIR}/TestList.c
	${TEST_DIR}/TestPrefs.c
	${TEST_DIR}/TestTimer.c
	${TEST_DIR}/TestLog.c
	${TEST_DIR}/TestUtils.c
	${TEST_DIR}/TestStopwatch.c
	${TEST_DIR}/TestError.c
	${TEST_DIR}/Tests.c
)

add_executable(libRingBuffer_tests ${TEST_SOURCES})

TARGET_LINK_LIBRARIES(libRingBuffer_tests
	RingBufferStatic
	pthread
)
