#include "bbt/redis/connect/AsyncCommand.hpp"

namespace bbt::database::redis
{

AsyncCommand::AsyncCommand(std::weak_ptr<AsyncConnection> connection, std::string&& cmd, OnReplyCallback cb):
    m_connection(connection),
    m_cmd(std::move(cmd)),
    m_on_reply_handler(cb)
{
}

AsyncCommand::~AsyncCommand()
{
}

void AsyncCommand::OnReply(RedisErrOpt err, std::shared_ptr<Reply> reply)
{
    if (m_on_reply_handler)
        m_on_reply_handler(err, reply);
}

RedisErrOpt AsyncCommand::DoCommand(redisAsyncContext* context, redisCallbackFn* fn)
{
    if (redisAsyncCommand(context, fn, this, m_cmd.c_str()) != REDIS_OK)
        return RedisErr(context->errstr, RedisErrType::Comm_UnDefErr);

    return std::nullopt;
}



} // namespace name
