cmake_minimum_required(VERSION 3.1)

project(bbt_redis)
enable_language(C CXX)

set(CXX_STANDARD C++17)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib) 
include_directories(
    ${PROJECT_SOURCE_DIR}
)

file(GLOB_RECURSE SRCS
    ${PROJECT_SOURCE_DIR}/bbt/*.cc
    ${PROJECT_SOURCE_DIR}/bbt/*.cc
)

add_library(bbt_redis SHARED ${SRCS})

target_link_libraries(bbt_redis
    event_core
    event_pthreads
    pthread
    hiredis
    ybbt
    bbt_network
)

add_subdirectory(example)