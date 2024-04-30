#include "bbt/redis/connect/AsyncConnection.hpp"
#include "bbt/redis/reply/Reply.hpp"
using namespace bbt::database::redis;

std::atomic_int num = 0;
std::atomic_int total_send = 0;
std::atomic_int send_err = 0;

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

void Thread(std::shared_ptr<AsyncConnection> conn)
{
    for (int j = 0; j < 5; ++j)
    {
        for (int i = 0; i < 10000; ++i) {

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
                printf("get value=%s, num = %d\n", value.c_str(), num++);
            });
            if (err != std::nullopt)
                send_err++;
            else
                total_send++;
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

    auto thread1 = new std::thread([=](){ Thread(conn); });
    auto thread2 = new std::thread([=](){ Thread(conn); });

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

    thread->Stop();
    if (thread1->joinable())
        thread1->join();
    if (thread2->joinable())
        thread2->join();
    printf("total=%d err=%d\n", total_send.load(), send_err.load());
    return 0;
}