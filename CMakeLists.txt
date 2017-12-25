cmake_minimum_required(VERSION 3.5)
project(misp)

set(CMAKE_CXX_STANDARD 14)

add_executable(misp
        misp.cpp)

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/core.mp
#        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/core.mp ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
