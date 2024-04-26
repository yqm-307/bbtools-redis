#include "Reply.hpp"
#include "Type.hpp"

namespace bbt::database::redis
{

Reply::Reply(redisReply* reply)
    :m_reply(reply)
{
    m_type = (redis::ReplyType)m_reply->type;
}

Reply::~Reply()
{
    Clear();
}

Reply::operator bool() const
{
    return (m_reply != nullptr);
}

void Reply::Clear()
{
    if (m_reply != nullptr)
        freeReplyObject(m_reply);

    m_reply = nullptr;
}

RedisErrOpt Reply::GetValue(int& value) const
{
    if (m_type != redis::ReplyType::kInteger) {
        return RedisErr("not is integer", err::RedisErrType::Reply_UnExpectedType);
    }

    value = m_reply->integer;

    return std::nullopt;
}

RedisErrOpt Reply::GetValue(std::string& value) const
{
    if (m_type != redis::ReplyType::kCString) {
        return RedisErr("not is string", err::RedisErrType::Reply_UnExpectedType);
    }

    value = m_reply->str;

    return std::nullopt;
}

RedisErrOpt Reply::GetValue(double& value) const
{
    if (m_type != redis::ReplyType::kDouble) {
        return RedisErr("not is double", err::RedisErrType::Reply_UnExpectedType);
    }

    value = m_reply->dval;

    return std::nullopt;
}



}