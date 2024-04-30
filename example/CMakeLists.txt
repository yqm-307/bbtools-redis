
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/example)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

add_executable(ConnectTest ConnectTest.cc)
target_link_libraries(ConnectTest bbt_redis)

add_executable(AsyncConnectTest AsyncConnectTest.cc)
target_link_libraries(AsyncConnectTest 
    bbt_redis
)

add_executable(AsyncConnectLibevent AsyncConnect.cc)
target_link_libraries(AsyncConnectLibevent 
    bbt_redis
    tcmalloc
)