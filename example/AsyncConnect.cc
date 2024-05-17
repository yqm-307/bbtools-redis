#include "bbt/redis/connect/AsyncConnection.hpp"
#include "bbt/redis/reply/Reply.hpp"
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


int main(int argc, char* argv[])
{
    /* 通过make_shared申请一个自动gc对象 */
    auto thread = std::make_shared<bbt::network::libevent::IOThread>(std::make_shared<bbt::network::libevent::EventLoop>());
    std::vector<std::thread*> threads{10};

    /* 创建一个AsyncConnect对象 */
    auto  conn = AsyncConnection::Create(thread, [](bbt::database::redis::RedisErrOpt err){
        perror(err.value().CWhat());
    });

    /* 创建一个线程，入参是线程运行时函数 */
    for (int i = 0; i < threads.size(); ++i) {
        threads[i] = new std::thread([=](){ Thread(conn); });
    }

    /* 发起一个异步连接 */
    conn->AsyncConnect(
        "127.0.0.1",
        6379,
        2000,
        5000,
        /* 连接建立完成的回调 */
        [](RedisErrOpt err, AsyncConnection* conn){ 
            if (err != std::nullopt) {
                printf("%s\n", err.value().CWhat());
            }
            OnConnect(conn);
        },
        /* 连接关闭时的回调 */
        [](auto _, AsyncConnection* conn){
        }
    );

    /* 启动 IOThread 线程 */
    thread->Start();

    /* 主线程休眠 */
    sleep(120);
    thread_running.exchange(false);

    /* 归还线程资源 */
    thread->Stop();
    for (auto&& thread : threads)  {
        if (thread->joinable())
            thread->join();
    }

    printf("total=%ld err=%ld onreply:%ld\n", total_send.load(), send_err.load(), num.load());
    return 0;
}