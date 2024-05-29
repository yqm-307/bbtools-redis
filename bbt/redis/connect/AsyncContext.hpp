#pragma once
#include <bbt/redis/Define.hpp>
#include <bbt/network/adapter/libevent/Network.hpp>

namespace bbt::database::redis
{

/**
 * 生命期和 hiredis 的redisContext一致
*/
class AsyncContext
{
public:
    AsyncContext(bbt::network::libevent::Network network, const bbt::errcode::OnErrorCallback& cb);
    ~AsyncContext();

    RedisErrOpt AsyncConnect(
        const std::string&  ip,
        short               port,
        int                 connect_timeout,
        int                 command_timeout,
        OnConnectCallback   onconn_cb,
        OnCloseCallback     onclose_cb
    );

protected:
    /* cfunction 传递参数时，weak ptr 包裹，更安全 */

    static void __CFuncOnConnect(const redisAsyncContext*, int status);
    static void __CFuncOnClose(const redisAsyncContext*, int status);
private:
    std::weak_ptr<bbt::network::libevent::IOThread>
                        m_conn_bind_thread;
    timeval             m_connect_timeout;
    timeval             m_command_timeout;
    redisAsyncContext*  m_redis_context;
    redisOptions        m_redis_option;

    std::function<void(std::shared_ptr<AsyncConnection>, RedisErrOpt)> 
                                        m_on_connect_callback{nullptr};
    std::function<void(RedisErrOpt)>    m_on_close_callback{nullptr};
    bbt::errcode::OnErrorCallback       m_on_error_callback{nullptr};
};

}