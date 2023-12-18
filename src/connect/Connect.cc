#include "Connect.hpp"

namespace bbt::database::redis
{

Connection::Connection(const std::string& ip, short port)
    :m_redis_address(ip, port)
{
}

Connection::~Connection()
{
    Disconnect();
}

void Connection::Disconnect()
{
    if (IsConnected())
        return;
    
    redisFree(m_raw_ctx);
}

RedisErrOpt Connection::ReConnect()
{
    if (IsConnected())
        return std::nullopt;
    
    return Connect();
}

RedisErrOpt Connection::Connect()
{
    m_raw_ctx = redisConnect(m_redis_address.GetIP().c_str(), m_redis_address.GetPort());

    if (m_raw_ctx == nullptr ) {
        return RedisErr("connect is null", RedisErrType::ConnectionFailed);
    } else if(m_raw_ctx->err != 0) {
        return RedisErr(m_raw_ctx->errstr, RedisErrType::ConnectionFailed);
    } else {
        return std::nullopt;
    }
}

std::pair<RedisErrOpt, Reply::SPtr> Connection::SyncExecCmd(std::vector<std::string> command)
{
    std::vector<const char*> args; 
    std::vector<size_t> lens;
    args.reserve(command.size());
    lens.reserve(command.size());
    for (int i=0; i<command.size(); ++i) {
        args[i] = command[i].c_str();
        lens[i] = command[i].size();
    }

    void* raw_reply = redisCommandArgv(m_raw_ctx, command.size(), args.data(), lens.data());

    return CheckReply(static_cast<redisReply*>(raw_reply));
}

std::pair<RedisErrOpt, Reply::SPtr> Connection::SyncExecCmd(const char* format, ...)
{
    std::lock_guard<std::mutex> lock(m_buf_lock);

    va_list va;

    va_start(va, format);
    vsnprintf(m_format_buffer, sizeof(m_format_buffer), format, va);
    va_end(va);

    SyncExecCmd(m_format_buffer);

    return ;
}

std::pair<RedisErrOpt, Reply::SPtr> Connection::SyncExecCmd(const std::string& command)
{
    void* raw_reply = redisCommand(m_raw_ctx, command.c_str());
    return CheckReply(static_cast<redisReply*>(raw_reply));
}

std::pair<RedisErrOpt, Reply::SPtr> Connection::CheckReply(redisReply* reply)
{
    if (reply == nullptr) {
        return {RedisErr("reply is null!", err::RedisErrType::Comm_ParamIsNull), nullptr};
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return {RedisErr(reply->str, err::RedisErrType::Reply_Err), nullptr};
    }

    return {std::nullopt, std::make_shared<Reply>(reply)};
}

}// namespace bbt::database::redis