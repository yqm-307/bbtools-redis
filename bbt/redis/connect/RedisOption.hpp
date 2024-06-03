#pragma once
#include <bbt/redis/Define.hpp>
#include <bbt/network/adapter/libevent/Network.hpp>
#include <bbt/redis/connect/AsyncContext.hpp>

namespace bbt::database::redis
{

/**
 * @brief 用于保存初始化一条redis连接的可选项
 */
class RedisOption
{
public:
    RedisOption(std::shared_ptr<bbt::network::libevent::IOThread> bind_thread, bbt::errcode::OnErrorCallback onerr);
    ~RedisOption();

    void SetRedisOptions(int opt);
    void SetConnectTimeout(int timeout);
    void SetCommandTimeout(int timeout);
    void SetTCP(const char* ip, short port);
    void SetOnConnect(const OnConnectCallback& on_conn_cb);
    void SetOnClose(const OnCloseCallback& on_close_cb);

    void OnConnect(RedisErrOpt err, std::shared_ptr<AsyncConnection> conn);
    void OnClose(RedisErrOpt err, bbt::net::IPAddress peer_addr);
    void OnError(const RedisErr& err);

    redisOptions* GetRawRedisOptions();
    std::shared_ptr<bbt::network::libevent::IOThread> GetBindThread();
private:
    std::weak_ptr<bbt::network::libevent::IOThread>
                        m_conn_bind_thread;
    timeval             m_connect_timeout;
    timeval             m_command_timeout;
    // redisAsyncContext*  m_context;
    redisOptions        m_raw_redis_option;
    OnConnectCallback   m_on_connect{nullptr};
    OnCloseCallback     m_on_close{nullptr};
    bbt::errcode::OnErrorCallback
                        m_on_error{nullptr};
};

}