cmake_minimum_required(VERSION 3.5)
project(PriorityQueues)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES my_pqueue.h my_pqueue.c queue_test.c)
add_executable(PriorityQueues ${SOURCE_FILES})