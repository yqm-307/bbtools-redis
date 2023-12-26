#pragma once
#include "Define.hpp"
#include "RedisConnection.hpp"
#include <bbt/net/IPAddress.hpp>

namespace bbt::database::redis
{

class AsyncConnection:
    public RedisConnection
{
public:
    explicit AsyncConnection(const std::string& ip = "127.0.0.1", short port = 6379);
    ~AsyncConnection();


    RedisErrOpt AsyncExecCmd(const std::string& command, const OnReplyCallback& cb);

    virtual RedisErrOpt Reconnect() override;
    virtual RedisErrOpt Disconnect() override;
    virtual bool IsConnected() override;

protected:

    RedisErrOpt Connect();

private:
    redisAsyncContext* m_raw_async_ctx{nullptr};
    redisContextFuncs m_io_funcs;
    bbt::net::IPAddress m_redis_addr;
};

} // namespace bbt::database::redis
