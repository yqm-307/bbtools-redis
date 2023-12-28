#pragma once
#include "Define.hpp"
#include "RedisConnection.hpp"
#include <bbt/net/IPAddress.hpp>

namespace bbt::database::redis
{


class AsyncConnection:
    // public RedisConnection,
    public std::enable_shared_from_this<AsyncConnection>
{
public:
    typedef struct { std::weak_ptr<AsyncConnection> ptr; } OnReplyUpValue;


    explicit AsyncConnection(const std::string& ip = "127.0.0.1", short port = 6379);
    ~AsyncConnection();


    RedisErrOpt AsyncExecCmd(const std::string& command, const OnReplyCallback& cb);

    RedisErrOpt Reconnect();
    RedisErrOpt Disconnect();
    bool IsConnected();


    void SetWriteHandler(const IOWriteHandler& cb) { m_write_handler = cb; }
    void SetReadHandler(const IOReadHandler& cb) { m_read_handler = cb; }

    static void OnReply(redisAsyncContext* ctx, void* rpy, void* udata);
    static void Send(redisContext* ctx);
    static void Recv(redisContext* ctx, char* buf, size_t size);

    /* 执行一次数据发送 */
    void DoNetSend();
    /* 执行一次数据接收 */
    void DoNetRecv();
protected:
    RedisErrOpt Connect();

private:
    redisAsyncContext* m_raw_async_ctx{nullptr};
    redisContextFuncs m_io_funcs;
    bbt::net::IPAddress m_redis_addr;

    IOWriteHandler  m_write_handler{nullptr};
    IOReadHandler   m_read_handler{nullptr};
};



} // namespace bbt::database::redis
