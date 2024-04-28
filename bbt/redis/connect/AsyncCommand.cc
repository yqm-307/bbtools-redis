#include "bbt/redis/connect/AsyncCommand.hpp"

namespace bbt::database::redis
{

AsyncCommand::AsyncCommand(std::weak_ptr<AsyncConnection> connection, OnReplyCallback cb):
    m_connection(connection)
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




} // namespace name
