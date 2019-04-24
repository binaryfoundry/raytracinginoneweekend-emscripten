cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME rtow)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(SRC_DIR "${PROJECT_SOURCE_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_STANDARD 11)

project(${PROJECT_NAME} C CXX)

file(GLOB SOURCES
    "*.cc")

file(GLOB HEADERS
    "*.h")

list (APPEND HEADERS ${HEADERS_proto})
list (APPEND SOURCES ${SOURCES_proto})

SOURCE_GROUP("Source" FILES ${SOURCES})
SOURCE_GROUP("Source" FILES ${HEADERS})

include_directories(
    ${PROJECT_SOURCE_DIR})

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS})

target_link_libraries(
    ${PROJECT_NAME})
