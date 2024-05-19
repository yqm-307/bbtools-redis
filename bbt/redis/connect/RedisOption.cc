#include <cstring>
#include <event2/util.h>
#include <bbt/redis/connect/RedisOption.hpp>
#include <bbt/base/assert/Assert.hpp>

#define SET_TIMEVAL(tv, ms) \
do { \
    evutil_timerclear(tv);\
    (tv)->tv_sec = ms / 1000;\
    (tv)->tv_usec = (ms % 1000) / 1000;\
} while(0);

namespace bbt::database::redis
{


void RedisOption::__CFuncOnConnect(const redisAsyncContext* ctx, int status)
{
    if (ctx->c.privdata == nullptr)
        return;

    auto priv = static_cast<RedisOptionPrivData*>(ctx->c.privdata);
    auto ptr = std::make_shared<AsyncConnection>(, ctx->c.fd);
    if (status == REDIS_OK)
        priv->m_on_connect(std::nullopt);
    else
        priv->m_on_connect(RedisErr(ctx->errstr, RedisErrType::ConnectionFailed));
}

void RedisOption::__CFuncOnClose(const redisAsyncContext* ctx, int status)
{
    if (ctx->c.privdata == nullptr)
        return;

    auto priv = static_cast<RedisOptionPrivData*>(ctx->c.privdata);
    if (status == REDIS_OK)
        priv->m_on_close(std::nullopt);
    else
        priv->m_on_close(RedisErr(ctx->errstr, RedisErrType::Comm_ParamErr));
}

RedisOption::RedisOption(std::shared_ptr<bbt::network::libevent::IOThread> bind_thread):
    m_conn_bind_thread(bind_thread)
{
    memset(&m_raw_redis_option, '\0', sizeof(m_raw_redis_option));
}

RedisOption::~RedisOption()
{

}

void RedisOption::SetRedisOptions(int opt)
{
    m_raw_redis_option.options |= opt;
}

void RedisOption::SetConnectTimeout(int timeout)
{
    AssertWithInfo(timeout > 0, "param error!");
    memset(&m_connect_timeout, '\0', sizeof(m_connect_timeout));
    SET_TIMEVAL(&m_connect_timeout, timeout);
}

void RedisOption::SetCommandTimeout(int timeout)
{
    AssertWithInfo(timeout > 0, "param error!");
    memset(&m_command_timeout, '\0', sizeof(m_command_timeout));
    SET_TIMEVAL(&m_command_timeout, timeout);
}

void RedisOption::SetTCP(const char* ip, short port)
{
    REDIS_OPTIONS_SET_TCP(&m_raw_redis_option, ip, port);
}

void RedisOption::SetOnConnect(const OnConnectCallback& on_conn_cb)
{
    m_on_connect = on_conn_cb;
}

void RedisOption::SetOnClose(const OnCloseCallback& on_close_cb)
{

}

redisAsyncContext* RedisOption::Connect()
{
    m_raw_redis_option = redisAsyncConnectWithOptions(&m_raw_redis_option);
}

}

#ifdef SET_TIMEVAL
#undef SET_TIMEVAL
#endif
