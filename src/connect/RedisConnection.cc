#include "RedisConnection.hpp"

namespace bbt::database::redis
{

RedisConnection::RedisConnection()
    :m_type(detail::RedisConnectionType::EM_Invalied)
{
}

RedisConnection::RedisConnection(detail::RedisConnectionType type)
    :m_type(type)
{
}

RedisConnection::~RedisConnection()
{
    m_type = detail::RedisConnectionType::EM_Invalied;
}


// void RedisConnection::SetOnConnectCallback(const RedisConnectionOnConnectCallback& cb)
// {
//     m_on_connect_callback = cb;
// }

// void RedisConnection::SetOnReadCallback(const RedisConnectionOnReadCallback& cb)
// {
//     m_on_read_callback = cb;
// }

// void RedisConnection::SetOnWriteCallback(const RedisConnectionOnWriteCallback& cb)
// {
//     m_on_write_callback = cb;
// }

// void RedisConnection::SetOnCloseCallback(const RedisConnectionOnCloseCallback& cb)
// {
//     m_on_close_callback = cb;
// }



} // namespace bbt::database::redis
