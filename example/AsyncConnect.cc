#include <bbt/base/hash/Hash.hpp>
#include "bbt/redis/connect/AsyncConnection.hpp"
#include "bbt/redis/reply/Reply.hpp"
#include "bbt/redis/RedisClient.hpp"
#include "bbt/network/adapter/libevent/Network.hpp"
#include "bbt/base/clock/Clock.hpp"

using namespace bbt::database::redis;

std::atomic_int64_t num = 0;
std::atomic_int64_t total_send = 0;
std::atomic_int64_t send_err = 0;
std::atomic_bool    thread_running = true;

void OnConnect(AsyncConnection* conn)
{
    conn->AsyncExecCmd("SET field1 10", nullptr);

    conn->AsyncExecCmd("GET field1", [](RedisErrOpt err, std::shared_ptr<Reply> reply){
        if (err != std::nullopt) {
            perror(err.value().CWhat());
            return;
        }

        std::string value;
        auto err1 = reply->Transform(value);
        if (err1 != std::nullopt)
            perror(err1.value().CWhat());
        printf("reply:%d\n", std::stoi(value));
    });
}

/* 测试案例发送线程 */
void Thread(std::shared_ptr<AsyncConnection> conn)
{
    while (thread_running)
    {
        for (int i = 0; i < 1000; ++i) {
            /* 发起redis异步指令 */
            auto err = conn->AsyncExecCmd("GET field1", [conn](RedisErrOpt err, std::shared_ptr<Reply> reply){
                if (err != std::nullopt) {
                    perror(err.value().CWhat());
                    return;
                }

                std::string value;
                auto err1 = reply->Transform(value);
                if (err1 != std::nullopt)
                    perror(err1.value().CWhat());

                conn->AsyncExecCmd("SET field1 " + std::to_string(std::stoi(value) + 1), nullptr);
                num++;
                // printf("get value=%s, num = %ld\n", value.c_str(), num++);
            });

            /* 测试计数 */
            if (err != std::nullopt)
                send_err++;
            else
                total_send++;
        }
        std::this_thread::sleep_until(bbt::timer::clock::nowAfter(bbt::timer::clock::seconds(1)));
    }
}

void Test1()
{
    bbt::network::libevent::Network network;
    RedisClient client{[](RedisErrOpt opt){
        perror(opt.value().CWhat());
    }};

    auto err = client.AsyncConnect(network, "127.0.0.1", 6379, 3000, 3000,
    [](RedisErrOpt error, std::shared_ptr<AsyncConnection> conn)
    {
        printf("新连接完成!\n");
    },
    [](RedisErrOpt, bbt::net::IPAddress addr)
    {
        printf("连接关闭! %s\n", addr.GetIPPort().c_str());
    });

    if (err != std::nullopt) {
        printf("建立连接发生错误 %s\n", err.value().CWhat());
        return;
    }

    std::this_thread::sleep_for(bbt::clock::seconds(2));
}

int main(int argc, char* argv[])
{
    /* 通过make_shared申请一个自动gc对象 */
    Test1();

}