/**
 * @file Test_async_connect.cc
 * @author your name (you@domain.com)
 * @brief 测试连接建立
 * @version 0.1
 * @date 2024-06-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <bbt/network/adapter/libevent/Network.hpp>
#include <boost/test/included/unit_test.hpp>

#include <bbt/redis/RedisClient.hpp>

BOOST_AUTO_TEST_SUITE(Async_Connect)

BOOST_AUTO_TEST_CASE(t_async_connect)
{
    bbt::network::libevent::Network network;
    auto err1 = network.AutoInitThread(2);
    BOOST_ASSERT_MSG(err1.IsErr(), err1.CWhat());
    network.StartListen();
}

BOOST_AUTO_TEST_SUITE_END()