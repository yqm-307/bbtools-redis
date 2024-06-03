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

RedisOption::RedisOption(std::shared_ptr<bbt::network::libevent::IOThread> bind_thread, bbt::errcode::OnErrorCallback onerr):
    m_on_error(onerr),
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

void RedisOption::OnConnect(RedisErrOpt err, std::shared_ptr<AsyncConnection> conn)
{
    if (m_on_connect == nullptr) {
        OnError(RedisErr{"onconnect callback is null!", RedisErrType::Comm_ParamErr});    
        return;
    }

    m_on_connect(err, conn);
}

void RedisOption::OnClose(RedisErrOpt err, bbt::net::IPAddress peer_addr)
{
    if (m_on_close == nullptr) {
        OnError(RedisErr{"onclose callback is null!", RedisErrType::Comm_ParamErr});
        return;
    }

    m_on_close(err, peer_addr);
}

void RedisOption::OnError(const RedisErr& err)
{
    AssertWithInfo(m_on_error != nullptr, "please set onerror handler!");
    m_on_error(err);
}

redisOptions* RedisOption::GetRawRedisOptions()
{
    return &m_raw_redis_option;
}

std::shared_ptr<bbt::network::libevent::IOThread> RedisOption::GetBindThread()
{
    return m_conn_bind_thread.lock();
}

}

#ifdef SET_TIMEVAL
#undef SET_TIMEVAL
#endif
