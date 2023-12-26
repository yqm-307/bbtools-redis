#include "AsyncConnection.hpp"
#include <hiredis/async.h>

namespace bbt::database::redis
{

void AsyncConnection::OnReply(redisAsyncContext* ctx, void* rpy, void* udata)
{

}


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

RedisErrOpt AsyncConnection::AsyncExecCmd(const std::string& command, const OnReplyCallback& cb)
{
    if (command.empty())
        return RedisErr("command is empty", RedisErrType::Comm_ParamIsNull);

    auto wptr = new WPtr();
    wptr->ptr =  weak_from_this();
    //TODO 解析errcode
    int redis_err = redisAsyncCommand(m_raw_async_ctx, &AsyncConnection::OnReply, (void*)wptr, command.c_str());

    redisAsyncWrite(m_raw_async_ctx);
}



} // namespace bbt::database::redis
