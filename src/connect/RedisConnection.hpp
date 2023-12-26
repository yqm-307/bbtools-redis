#pragma once
#include "Define.hpp"

namespace bbt::database::redis
{

namespace detail
{
enum RedisConnectionType
{
    EM_Invalied = 0,
    EM_Async = 1,
    EM_Sync = 2,
};
}


/**
 * @brief redis connection的基类
 * 
 */
class RedisConnection
{
public:
    ~RedisConnection();

protected:
    RedisConnection();
    explicit RedisConnection(detail::RedisConnectionType type);

    virtual RedisErrOpt Disconnect() = 0;
    virtual RedisErrOpt Reconnect() = 0;
    virtual bool IsConnected() = 0;
protected: /* friend */
    // void SetOnConnectCallback(const RedisConnectionOnConnectCallback& cb);
    // void SetOnReadCallback(const RedisConnectionOnReadCallback& cb);
    // void SetOnWriteCallback(const RedisConnectionOnWriteCallback& cb);
    // void SetOnCloseCallback(const RedisConnectionOnCloseCallback& cb);

protected:
    virtual ssize_t Send(const char* buffer, size_t len) = 0;
    virtual ssize_t Recv(const char* bytes, size_t len) = 0;
private:
    detail::RedisConnectionType m_type;
    // RedisConnectionOnCloseCallback m_on_close_callback{nullptr};
    // RedisConnectionOnConnectCallback m_on_connect_callback{nullptr};
    // RedisConnectionOnReadCallback m_on_read_callback{nullptr};
    // RedisConnectionOnWriteCallback m_on_write_callback{nullptr};
};

} // namespace bbt::database::redis
