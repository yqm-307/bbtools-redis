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
    friend class RedisOption;

    AsyncContext(std::shared_ptr<RedisOption> opt);
    ~AsyncContext();

    RedisErrOpt AsyncConnect();

    template<typename ...Args>
    RedisErrOpt AsyncCommand(redisCallbackFn *fn, void *privdata, const char *format, Args ...args)
    {
        if (redisAsyncCommand(m_async_context, fn, privdata, format, args ...) != REDIS_OK)
            return RedisErr(m_async_context->errstr, RedisErrType::Comm_UnDefErr);
        return std::nullopt;
    }

    std::shared_ptr<bbt::network::libevent::IOThread> GetBindThread();

    RedisErrOpt         SetCommandTimeout(int timeout);
    bbt::net::IPAddress GetPeerAddress();

    int  GetSocketFd();
    void Close();
    void OnError(const RedisErr& err);
protected:
    void OnConnect(RedisErrOpt err, std::shared_ptr<AsyncConnection> conn);
    void OnClose(RedisErrOpt err, bbt::net::IPAddress conn);
    static void __CFuncOnConnect(const redisAsyncContext* ctx, int status);
    static void __CFuncOnClose(const redisAsyncContext* ctx, int status);
private:
    std::shared_ptr<AsyncConnection> m_conn{nullptr};
    bbt::net::IPAddress              m_peer_addr;
    redisAsyncContext*               m_async_context{nullptr};
    std::shared_ptr<RedisOption>     m_redis_opt{nullptr};
};

}