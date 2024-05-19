#include <bbt/redis/connect/AsyncContext.hpp>
#include <bbt/redis/connect/AsyncConnection.hpp>

#define SET_TIMEVAL(tv, ms) \
do { \
    evutil_timerclear(tv);\
    (tv)->tv_sec = ms / 1000;\
    (tv)->tv_usec = (ms % 1000) / 1000;\
} while(0);


namespace bbt::database::redis
{

AsyncContext::AsyncContext(bbt::network::libevent::Network network)
{
    memset(&m_redis_option,     '\0',   sizeof(m_redis_option));
    memset(&m_redis_context,        '\0',   sizeof(m_redis_context));
}

AsyncContext::~AsyncContext()
{

}

RedisErrOpt AsyncContext::AsyncConnect(
    const std::string&  ip,
    short               port,
    int                 connect_timeout,
    int                 command_timeout,
    OnConnectCallback   onconn_cb,
    OnCloseCallback     onclose_cb)
{
    using namespace bbt::network::libevent;

    if (connect_timeout <= 0)
        return RedisErr{"connect timeout less then 0!", RedisErrType::Comm_ParamErr};

    if (command_timeout <= 0)
        return RedisErr{"command timeout less then 0!", RedisErrType::Comm_ParamErr};

    REDIS_OPTIONS_SET_TCP(&m_redis_option, ip.c_str(), port);
    m_redis_option.options |= REDIS_OPT_NONBLOCK;
    m_redis_option.options |= REDIS_OPT_REUSEADDR;
    m_redis_option.options |= REDIS_OPT_NOAUTOFREE;
    m_redis_option.options |= REDIS_OPT_NOAUTOFREEREPLIES;

    SET_TIMEVAL(&m_connect_timeout, connect_timeout);
    SET_TIMEVAL(&m_command_timeout, command_timeout);
    m_redis_option.connect_timeout = &m_connect_timeout;
    m_redis_option.command_timeout = &m_command_timeout;

    m_redis_context = redisAsyncConnectWithOptions(&m_redis_option);
    
    if (m_redis_context == nullptr)
        return RedisErr{"redisAsyncConnectWithOptions() failed!", RedisErrType::ConnectionFailed};
    else if (m_redis_context->err != 0)
        return RedisErr{m_redis_context->errstr, RedisErrType::ConnectionFailed};
    
    auto thread_ptr = m_conn_bind_thread.lock();
    if (thread_ptr == nullptr)
        return RedisErr{"bind thread is stopped!", RedisErrType::ConnectionFailed};

    event_base* base = thread_ptr->GetEventLoop()->GetEventBase()->GetRawBase();
    if (base == nullptr)
        return RedisErr{"unexpected error! event_base is nullptr!", RedisErrType::Comm_UnDefErr};
    
    if (redisLibeventAttach(m_redis_context, base) != REDIS_OK)
        return RedisErr{m_redis_context->errstr, RedisErrType::Hiredis_Default};
    
    if (redisAsyncSetConnectCallback(m_redis_context, __CFuncOnConnect) != REDIS_OK)
        return RedisErr{m_redis_context->errstr, RedisErrType::Hiredis_Default};
    
    if (redisAsyncSetDisconnectCallback(m_redis_context, __CFuncOnClose) != REDIS_OK)
        return RedisErr{m_redis_context->errstr, RedisErrType::Hiredis_Default};
    
    auto safe_this = new AsyncContextCFuncSafeParam{.m_safe_wkptr = weak_from_this()};
    REDIS_OPTIONS_SET_PRIVDATA(&(m_redis_context->c), safe_this, NULL);

    return std::nullopt;
}

void AsyncContext::__CFuncOnConnect(const redisAsyncContext* context, int status)
{
    if (context == nullptr)
        return;

    auto async_context_safe_param = static_cast<AsyncContextCFuncSafeParam*>(context->c.privdata);
    auto weak_this = async_context_safe_param->m_safe_wkptr;
    if (weak_this.expired())
        return;

    auto pthis = weak_this.lock();
    if (pthis == nullptr)
        return;

    auto new_redis_conn = AsyncConnection::Create(pthis->m_conn_bind_thread, [=](RedisErrOpt err){ OnErrCallback(err); });
    if (status == REDIS_OK) {
        pthis->m_on_connect();
    } else {

    }
}

void AsyncContext::__CFuncOnClose(const redisAsyncContext* context, int status)
{

}


}

#ifdef SET_TIMEVAL
#undef SET_TIMEVAL
#endif
