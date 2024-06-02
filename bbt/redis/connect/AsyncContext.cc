#include <bbt/redis/connect/AsyncContext.hpp>
#include <bbt/redis/connect/AsyncConnection.hpp>
#include <bbt/redis/connect/RedisOption.hpp>

#define SET_TIMEVAL(tv, ms) \
do { \
    evutil_timerclear(tv);\
    (tv)->tv_sec = ms / 1000;\
    (tv)->tv_usec = (ms % 1000) / 1000;\
} while(0);


namespace bbt::database::redis
{


void AsyncContext::__CFuncOnConnect(const redisAsyncContext* ctx, int status)
{
    if (ctx->c.privdata == nullptr)
        return;

    auto context = static_cast<AsyncContext*>(ctx->c.privdata);
    auto conn_bind_thread = context->GetBindThread();
    if (conn_bind_thread == nullptr)
        context->OnConnect(RedisErr{"bind thread is null!", RedisErrType::ConnectionFailed}, nullptr);

    auto conn_ptr = AsyncConnection::Create(context);
    if (status == REDIS_OK)
        context->OnConnect(std::nullopt, conn_ptr);
    else
        context->OnConnect(RedisErr(ctx->errstr, RedisErrType::ConnectionFailed), nullptr);
}

void AsyncContext::__CFuncOnClose(const redisAsyncContext* ctx, int status)
{
    if (ctx->c.privdata == nullptr)
        return;

    auto context = static_cast<AsyncContext*>(ctx->c.privdata);
    if (status == REDIS_OK)
        context->OnClose(std::nullopt, context->m_peer_addr);
    else
        context->OnClose(RedisErr(ctx->errstr, RedisErrType::Comm_ParamErr), context->m_peer_addr);
}

AsyncContext::AsyncContext(std::shared_ptr<RedisOption> opt):
    m_redis_opt(opt)
{
    memset(&m_async_context, '\0', sizeof(m_async_context));
}

AsyncContext::~AsyncContext()
{

}

RedisErrOpt AsyncContext::AsyncConnect()
{
    Assert(m_async_context == nullptr);
    m_async_context = redisAsyncConnectWithOptions(m_redis_opt->GetRawRedisOptions());

    if (m_async_context == nullptr)
        return RedisErr("new connection failed!", RedisErrType::ConnectionFailed);
    
    if (m_async_context->err != 0)
        return RedisErr(m_async_context->errstr, RedisErrType::ConnectionFailed);

    auto thread_sptr = m_redis_opt->GetBindThread();
    if (thread_sptr == nullptr)
        return RedisErr{"bind thread is null!", RedisErrType::ConnectionFailed};

    REDIS_OPTIONS_SET_PRIVDATA(&(m_async_context->c), this, NULL);
    if (m_async_context->err != 0)
        return RedisErr(m_async_context->errstr, RedisErrType::ConnectionFailed);

    auto base = thread_sptr->GetEventLoop()->GetEventBase()->GetRawBase();
    if (redisLibeventAttach(m_async_context, base) != REDIS_OK)
        return RedisErr{m_async_context->errstr, RedisErrType::ConnectionFailed};

    if (redisAsyncSetConnectCallback(m_async_context, __CFuncOnConnect) != REDIS_OK)
        return RedisErr{m_async_context->errstr, RedisErrType::ConnectionFailed};
    
    if (redisAsyncSetDisconnectCallback(m_async_context, __CFuncOnClose) != REDIS_OK)
        return RedisErr{m_async_context->errstr, RedisErrType::ConnectionFailed};
    
    return std::nullopt;
}

void AsyncContext::OnConnect(RedisErrOpt err, std::shared_ptr<AsyncConnection> conn)
{
    if (conn != nullptr) {
        m_peer_addr = conn->GetPeerAddress();
    }

    m_redis_opt->OnConnect(err, conn);
}

void AsyncContext::OnClose(RedisErrOpt err, bbt::net::IPAddress addr)
{
    m_redis_opt->OnClose(err, m_peer_addr);
}

void AsyncContext::OnError(const RedisErr& err)
{
    m_redis_opt->OnError(err);
}

void AsyncContext::Close()
{
    redisAsyncDisconnect(m_async_context);
}

std::shared_ptr<bbt::network::libevent::IOThread> AsyncContext::GetBindThread()
{
    return m_redis_opt->GetBindThread();
}

int AsyncContext::GetSocketFd()
{
    return m_async_context->c.fd;
}

bbt::net::IPAddress AsyncContext::GetPeerAddress()
{
    return m_peer_addr;
}

RedisErrOpt AsyncContext::SetCommandTimeout(int timeout)
{
    timeval command_timeout;

    if (timeout <= 0)
        return RedisErr("timeout less then 0!", RedisErrType::Comm_ParamErr);
    
    SET_TIMEVAL(&command_timeout, timeout);
    if (redisAsyncSetTimeout(m_async_context, command_timeout) != REDIS_OK) {
        return RedisErr(m_async_context->errstr, RedisErrType::Comm_OOM);
    }

    return std::nullopt;
}


} // namespace bbt::database::redis
#ifdef SET_TIMEVAL
#undef SET_TIMEVAL
#endif
