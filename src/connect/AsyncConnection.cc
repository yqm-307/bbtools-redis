#include "AsyncConnection.hpp"
#include <hiredis/async.h>

namespace bbt::database::redis
{

AsyncConnection::AsyncConnection(const std::string& ip, short port)
    :m_redis_addr(ip, port)
{
}

AsyncConnection::~AsyncConnection()
{
    Disconnect();
}

RedisErrOpt AsyncConnection::Reconnect()
{
    if (IsConnected())
        return std::nullopt;
    
    return Connect();
}

RedisErrOpt AsyncConnection::Connect()
{
    m_raw_async_ctx = redisAsyncConnect(m_redis_addr.GetIP().c_str(), m_redis_addr.GetPort());

    if (m_raw_async_ctx == nullptr) {
        return RedisErr("connect is null", RedisErrType::ConnectionFailed);
    } else if (m_raw_async_ctx->err != 0) {
        return RedisErr(m_raw_async_ctx->errstr, RedisErrType::ConnectionFailed);
    } else {
        return std::nullopt;
    }
}

RedisErrOpt AsyncConnection::Disconnect()
{
    if (!IsConnected())
        return;

    redisAsyncDisconnect(m_raw_async_ctx);
    redisAsyncFree(m_raw_async_ctx);
    m_raw_async_ctx = nullptr;
}

bool AsyncConnection::IsConnected()
{
    return (m_raw_async_ctx != nullptr);
}


} // namespace bbt::database::redis
