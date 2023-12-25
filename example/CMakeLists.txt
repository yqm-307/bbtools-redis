
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/example)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
)
add_executable(ConnectTest ConnectTest.cc)
target_link_libraries(ConnectTest redishelper)