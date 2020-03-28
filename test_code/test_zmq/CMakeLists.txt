cmake_minimum_required(VERSION 3.3)
project(ZmqProject)

# This will file libzmq.so file from /usr/local/lib
FIND_FILE(ZMQLIB libzmq.so /usr/local/lib)
FIND_FILE(PROTOC libprotoc.so /usr/local/lib)
FIND_FILE(PROTOBUF libprotobuf.so /usr/local/lib)
IF(NOT ZMQLIB)
    MESSAGE(SEND_ERROR "Ah.. Cannot find library libzmq.so.")
ENDIF(NOT ZMQLIB)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -I/usr/local/include")
set(SOURCE_FILES test_zmq.cpp detect_pipe.pb.cc)

add_executable(ZmqProject ${SOURCE_FILES})
# The following line will link with libzmq.so
TARGET_LINK_LIBRARIES( ZmqProject ${ZMQLIB} ${PROTOC} ${PROTOBUF})
