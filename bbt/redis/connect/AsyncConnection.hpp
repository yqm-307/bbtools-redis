#pragma once
#include <memory>
#include <hiredis/adapters/libevent.h>
#include <bbt/base/net/IPAddress.hpp>
#include <bbt/network/adapter/libevent/EventBase.hpp>
#include <bbt/network/adapter/libevent/EventLoop.hpp>
#include "bbt/redis/Define.hpp"
#include "bbt/redis/connect/RedisConnection.hpp"

namespace bbt::database::redis
{

class AsyncConnection;

struct CommandContext
{
    std::weak_ptr<AsyncConnection> redis_connection; // 所属连接
    OnReplyCallback                on_reply_handler{nullptr};
};

//TODO 改造为 libevent adapters
class AsyncConnection:
    // public RedisConnection,
    public std::enable_shared_from_this<AsyncConnection>
{
public:
    static std::shared_ptr<AsyncConnection> Create(bbt::network::libevent::EventBase* io_ctx, OnErrCallback onerr_cb);
    ~AsyncConnection();

    RedisErrOpt AsyncConnect(
        const std::string&  ip,
        short               port,
        int                 connect_timeout,
        int                 command_timeout,
        OnConnectCallback   onconn_cb,
        OnCloseCallback     onclose_cb);

    /* 执行异步指令 */
    RedisErrOpt AsyncExecCmd(const std::string& command, const OnReplyCallback& cb);

    /* 连接 */
    RedisErrOpt Reconnect();
    RedisErrOpt Disconnect();
    bool        IsConnected() const;
    void        Close();

    /* 取当前连接的socket fd，连接异常返回-1 */
    redisFD     GetSocket() const;
    RedisErrOpt SetCommandTimeout(int timeout);
    ConnId      GetConnId() const;
protected:
    explicit    AsyncConnection(bbt::network::libevent::EventBase* io_ctx, OnErrCallback onerr_cb);
    RedisErrOpt Connect();

    void        __InitPrivData();
    void        OnConnect(RedisErrOpt err);
    void        OnError(RedisErrOpt err);
    void        OnClose(RedisErrOpt err);

    /* 私有数据 */
    struct AsyncConnectionPrivData
    {
        std::function<void(RedisErrOpt)> m_on_connect;
        std::function<void(RedisErrOpt)> m_on_close;
    };

    static void __CFuncOnConnect(const redisAsyncContext* ctx, int status);
    static void __CFuncOnClose(const redisAsyncContext* ctx, int status);
    static void __OnReply(redisAsyncContext* ctx, void* rpy, void* udata);

private:
    std::shared_ptr<bbt::network::libevent::EventBase> m_io_ctx{nullptr};
    AsyncConnectionPrivData     m_priv_data{nullptr};
    redisAsyncContext*          m_raw_async_ctx{nullptr};
    redisContextFuncs           m_io_funcs;
    bbt::net::IPAddress         m_redis_addr;
    redisOptions                m_redis_conn_option;
    struct timeval              m_connect_timeout;  // connect() 超时时间
    struct timeval              m_command_timeout;  // 执行指令，超时时间
    volatile bool               m_is_connected{false};

    OnConnectCallback           m_on_connect_handler{nullptr};
    IOWriteHandler              m_write_handler{nullptr};
    IOReadHandler               m_read_handler{nullptr};
    OnErrCallback               m_on_err_handler{nullptr};
    OnCloseCallback             m_on_close_handler{nullptr};

    const ConnId                m_conn_id;
    static ConnId               m_current_id;
};



} // namespace bbt::database::redis
