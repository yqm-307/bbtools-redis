set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/unit_test)

set(BOOST_TEST_LIB
    boost_unit_test_framework
    boost_test_exec_monitor
)

add_executable(Test_async_connect Test_async_connect.cc)
target_link_libraries(Test_async_connect
    ${BOOST_TEST_LIB}
    bbt_redis
)
add_test(NAME Test_async_connect COMMAND Test_async_connect)


add_executable(Test_async_command Test_async_command.cc)
target_link_libraries(Test_async_command
    ${BOOST_TEST_LIB}
    bbt_redis
)
add_test(NAME Test_async_command COMMAND Test_async_command)