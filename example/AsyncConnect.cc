#include "bbt/redis/connect/AsyncConnection.hpp"
#include "bbt/redis/reply/Reply.hpp"
using namespace bbt::database::redis;

std::atomic_int num = 0;

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

void Thread(AsyncConnection* conn)
{
    for (int j = 0; j < 5; ++j)
    {
        for (int i = 0; i < 10000; ++i) {

            conn->AsyncExecCmd("GET field1", [conn](RedisErrOpt err, std::shared_ptr<Reply> reply){
                if (err != std::nullopt) {
                    perror(err.value().CWhat());
                    return;
                }

                std::string value;
                auto err1 = reply->Transform(value);
                if (err1 != std::nullopt)
                    perror(err1.value().CWhat());

                conn->AsyncExecCmd("SET field1 " + std::to_string(std::stoi(value) + 1), nullptr);
            });
        }
        std::this_thread::sleep_until(bbt::timer::clock::nowAfter(bbt::timer::clock::seconds(1)));
    }    
}


int main()
{
    auto thread = std::make_shared<bbt::network::libevent::IOThread>();

    auto  conn = AsyncConnection::Create(thread, [](bbt::database::redis::RedisErrOpt err){
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

    // loop->StartLoop(bbt::network::libevent::EventLoopOpt::LOOP_NO_EXIT_ON_EMPTY);
    thread->Start();

    sleep(10);
    return 0;
}