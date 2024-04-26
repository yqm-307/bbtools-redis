#include "bbt/redis/connect/AsyncConnection.hpp"
#include "bbt/redis/reply/Reply.hpp"
using namespace bbt::database::redis;

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

int main()
{
    auto* base = new bbt::network::libevent::EventBase();
    auto* loop = new bbt::network::libevent::EventLoop(base);
    auto  conn = AsyncConnection::Create(base, [](bbt::database::redis::RedisErrOpt err){
        perror(err.value().CWhat());
    });

    conn->AsyncConnect("127.0.0.1", 6379, 2000, 5000,
    [](RedisErrOpt err, AsyncConnection* conn){ 
        if (err != std::nullopt) {
            printf("%s\n", err.value().CWhat());
        }
        OnConnect(conn);
    },
    [](auto _, AsyncConnection* conn){});

    loop->StartLoop(bbt::network::libevent::EventLoopOpt::LOOP_NO_EXIT_ON_EMPTY);
    return 0;
}