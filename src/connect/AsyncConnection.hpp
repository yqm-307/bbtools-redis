#pragma once
#include "Define.hpp"
#include "RedisConnection.hpp"
#include <bbt/base/net/IPAddress.hpp>

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

    /* 执行异步指令 */
    RedisErrOpt AsyncExecCmd(const std::string& command, const OnReplyCallback& cb);

    /* 连接 */
    RedisErrOpt Reconnect();
    RedisErrOpt Disconnect();
    bool IsConnected();

    /* 设置 network io 函数 */
    // void SetWriteHandler(const IOWriteHandler& cb) { m_write_handler = cb; }
    // void SetReadHandler(const IOReadHandler& cb) { m_read_handler = cb; }

    static void OnReply(redisAsyncContext* ctx, void* rpy, void* udata);
    // static void Send(redisContext* ctx);
    // static void Recv(redisContext* ctx, char* buf, size_t size);

    /**
     * 当连接上的socket可写或者缓冲区有待发送数据，
     * 调用此函数来将此连接缓存的数据发送到redis
     * 服务端。
     */
    void OnSend();
    /**
     * 当连接上的socket可读，则调用此函数来将redis
     * 服务端传回的响应加载到连接的缓存中，并调用回
     * 调处理reply
     */
    void OnRecv();

    /* 取当前连接的socket fd，连接异常返回-1 */
    redisFD GetSocket();
protected:
    RedisErrOpt Connect();

private:
    redisAsyncContext* m_raw_async_ctx{nullptr};
    redisContextFuncs m_io_funcs;
    bbt::net::IPAddress m_redis_addr;

    IOWriteHandler  m_write_handler{nullptr};
    IOReadHandler   m_read_handler{nullptr};
    OnReplyCallback m_on_reply_handler{nullptr};
};



} // namespace bbt::database::redis
