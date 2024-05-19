#pragma once
#include <bbt/redis/Define.hpp>
#include <bbt/network/adapter/libevent/Network.hpp>

namespace bbt::database::redis
{

class AsyncContext:
    public std::enable_shared_from_this<AsyncContext>
{
public:
    typedef std::shared_ptr<AsyncContext> SPtr;
    typedef std::weak_ptr<AsyncContext>   WKPtr;

    static SPtr Create(bbt::network::libevent::Network network, const bbt::errcode::OnErrorCallback<bbt::errcode::>& cb);
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
    AsyncContext(bbt::network::libevent::Network network);
    /* cfunction 传递参数时，weak ptr 包裹，更安全 */
    struct AsyncContextCFuncSafeParam { AsyncContext::WKPtr m_safe_wkptr; };

    static void __CFuncOnConnect(const redisAsyncContext*, int status);
    static void __CFuncOnClose(const redisAsyncContext*, int status);
private:
    std::weak_ptr<bbt::network::libevent::IOThread>
                        m_conn_bind_thread;
    timeval             m_connect_timeout;
    timeval             m_command_timeout;
    redisAsyncContext*  m_redis_context;
    redisOptions        m_redis_option;
    std::function<void(std::shared_ptr<AsyncConnection>, RedisErrOpt)> m_on_connect;
    std::function<void(RedisErrOpt)> m_on_close;
};

}