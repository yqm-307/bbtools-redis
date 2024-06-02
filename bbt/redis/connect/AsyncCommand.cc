#include "bbt/redis/connect/AsyncCommand.hpp"
#include "bbt/redis/connect/AsyncContext.hpp"

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

RedisErrOpt AsyncCommand::DoCommand(AsyncContext* context, redisCallbackFn* cmd_done_callback)
{
    if (context == nullptr)
        return RedisErr{"context is null!", RedisErrType::Comm_ParamErr};

    return context->AsyncCommand(cmd_done_callback, this, m_cmd.c_str());
}



} // namespace name
