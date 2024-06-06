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
#include <boost/test/included/unit_test.hpp>

#include <bbt/network/adapter/libevent/Network.hpp>
#include <bbt/redis/RedisClient.hpp>
using namespace bbt::database::redis;

bbt::network::libevent::Network* g_network = nullptr;

struct NetworkFixture
{
    NetworkFixture() {Init();}
    ~NetworkFixture() { Destroy(); }
    void Init()
    {
        BOOST_ASSERT(bbt::network::GlobalInit());
        g_network = new bbt::network::libevent::Network();
        auto err1 = g_network->AutoInitThread(2);
        BOOST_CHECK_MESSAGE(!err1.IsErr(), err1.CWhat());
        auto err2 = g_network->StartListen("127.0.0.1", 10010, [](auto, auto){});
        BOOST_CHECK_MESSAGE(!err2.IsErr(), err2.CWhat());
        BOOST_ASSERT(!err2.IsErr());
        g_network->Start();
    }

    void Destroy()
    {
        delete g_network;
        g_network = nullptr;
    }
};

BOOST_AUTO_TEST_SUITE(Async_Connect)

BOOST_FIXTURE_TEST_CASE(t_single_async_connect, NetworkFixture)
{
    std::atomic_bool connect_succ = false;
    std::atomic_bool close_succ = false;
    RedisClient client{[](RedisErrOpt err){ printf("[OnError] %s\n", err.value().CWhat()); }};

    auto err3 = client.AsyncConnect(*g_network, "127.0.0.1", 6379, 3000, 3000,
    [&](auto error, auto conn_sptr){ connect_succ.exchange(true); },
    [&](auto err, bbt::net::IPAddress address){ BOOST_ASSERT(err == std::nullopt); });

    if (err3 != std::nullopt)
        BOOST_ASSERT_MSG(false, err3.value().CWhat());

    sleep(1);
    g_network->Stop();
    BOOST_CHECK(connect_succ.load());
}

BOOST_FIXTURE_TEST_CASE(t_multi_async_connect, NetworkFixture)
{
    const int test_num = 100;
    std::atomic_int connect_succ_count = 0;
    std::atomic_int connect_close_succ_count = 0;

    RedisClient client{[](RedisErrOpt err){ BOOST_WARN_MESSAGE("[OnConnect] %s", err.value().CWhat()); }};

    for (int i = 0; i < test_num; ++i)
    {
        auto err3 = client.AsyncConnect(*g_network, "127.0.0.1", 6379, 3000, 3000,
        [&](auto error, auto conn_sptr){ connect_succ_count++; },
        [&](auto err, bbt::net::IPAddress address){ BOOST_ASSERT(err == std::nullopt); connect_close_succ_count++; });

        if (err3 != std::nullopt)
            BOOST_ASSERT_MSG(false, err3.value().CWhat());
    }

    sleep(3);
    g_network->Stop();
    BOOST_CHECK_EQUAL(connect_close_succ_count.load(), test_num);
    BOOST_CHECK_EQUAL(connect_succ_count.load(), test_num);
}

BOOST_AUTO_TEST_SUITE_END()