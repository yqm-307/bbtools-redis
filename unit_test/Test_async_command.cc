#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <bbt/network/adapter/libevent/Network.hpp>
#include <bbt/redis/RedisClient.hpp>
#include <bbt/redis/reply/Reply.hpp>
using namespace bbt::database::redis;

RedisClient* g_client = nullptr;
bbt::network::libevent::Network* g_network = nullptr;

struct RedisClientFixture
{
    RedisClientFixture()
    {
        g_network = new bbt::network::libevent::Network();
        BOOST_ASSERT(bbt::network::GlobalInit());
        auto err1 = g_network->AutoInitThread(2);
        BOOST_CHECK_MESSAGE(!err1.IsErr(), err1.CWhat());
        auto err2 = g_network->StartListen("127.0.0.1", 10010, [](auto, auto){});
        BOOST_CHECK_MESSAGE(!err2.IsErr(), err2.CWhat());
        BOOST_ASSERT(!err2.IsErr());
        g_network->Start();

        g_client = new RedisClient([](const auto& err){ BOOST_ASSERT_MSG(!err.IsErr(), err.CWhat()); });
    }

    ~RedisClientFixture()
    {
        delete g_client;
        g_client = nullptr;

        delete g_network;
        g_network = nullptr;
    }
};

BOOST_AUTO_TEST_SUITE(Async_Command)

BOOST_FIXTURE_TEST_CASE(t_single_async_do_command, RedisClientFixture)
{
    bbt::thread::CountDownLatch downlatch_async_connect{1};
    bbt::thread::CountDownLatch downlatch_async_command_set{1};
    bbt::thread::CountDownLatch downlatch_async_command_get{1};
    std::shared_ptr<AsyncConnection> redis_connection = nullptr;
    std::string field = "bbt_redis_test_field";
    const std::string field_value = "hello_world";

    BOOST_ASSERT(g_client != nullptr);
    auto err1 = g_client->AsyncConnect(*g_network, "127.0.0.1", 6379, 2000, 2000,
    [&](RedisErrOpt opt, std::shared_ptr<AsyncConnection> conn){
        if (opt != std::nullopt)
            BOOST_ASSERT_MSG(false, opt.value().CWhat());

        redis_connection = conn;
        downlatch_async_connect.Down();
    },
    [](RedisErrOpt opt, const bbt::net::IPAddress& addr){ BOOST_ASSERT(!opt.has_value()); });

    if (err1 != std::nullopt)
        BOOST_ASSERT_MSG(false, err1.value().CWhat());

    downlatch_async_connect.Wait();

    redis_connection->AsyncExecCmd("Set " + field + ' ' + field_value,
    [&](RedisErrOpt opt, std::shared_ptr<Reply> reply){
        if (opt != std::nullopt)
            BOOST_ASSERT_MSG(false, opt.value().CWhat());
        
        BOOST_ASSERT_MSG(reply, "reply is null");
        downlatch_async_command_set.Down();
    });

    downlatch_async_command_set.Wait();

    redis_connection->AsyncExecCmd("Get " + field ,
    [&](RedisErrOpt opt, std::shared_ptr<Reply> reply){
        if (opt != std::nullopt)
            BOOST_ASSERT_MSG(false, opt.value().CWhat());
        
        BOOST_ASSERT_MSG(reply, "reply is null");
        std::string value;
        auto err = reply->Transform(value);
        if (err != std::nullopt)
            BOOST_ASSERT_MSG(false, err.value().CWhat());
        BOOST_CHECK_EQUAL(value, field_value);
        downlatch_async_command_get.Down();
    });

    downlatch_async_command_get.Wait();
}

BOOST_AUTO_TEST_SUITE_END()