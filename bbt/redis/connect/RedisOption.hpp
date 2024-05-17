#pragma once
#include <bbt/redis/Define.hpp>
#include <bbt/network/adapter/libevent/Network.hpp>

namespace bbt::database::redis
{

class RedisOption
{
public:
    RedisOption(std::shared_ptr<bbt::network::libevent::IOThread> bind_thread);
    ~RedisOption();

    void SetRedisOptions(int opt);
    void SetConnectTimeout(int timeout);
    void SetCommandTimeout(int timeout);
    void SetTCP(const char* ip, short port);
    void SetOnConnect(const OnConnectCallback& on_conn_cb);
    void SetOnClose(const OnCloseCallback& on_close_cb);
protected:
    /* cfunc wapper */
    static void __CFuncOnConnect(const redisAsyncContext* ctx, int status);
    static void __CFuncOnClose(const redisAsyncContext* ctx, int status);

private:
    std::weak_ptr<bbt::network::libevent::IOThread>
                        m_conn_bind_thread;
    timeval             m_connect_timeout;
    timeval             m_command_timeout;
    redisOptions        m_raw_redis_option;
    std::function<void(std::shared_ptr<AsyncConnection>, RedisErrOpt)> m_on_connect;

    std::function<void(RedisErrOpt)> m_on_close;
};

}