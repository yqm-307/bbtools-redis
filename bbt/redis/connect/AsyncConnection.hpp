#pragma once
#include "bbt/redis/connect/AsyncCommand.hpp"


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
    /**
     * @brief 创建一个自动gc的AsyncConnection对象
     * 
     * @param thread 连接运行的线程
     * @param onerr_cb 错误抛出函数
     * @return std::shared_ptr<AsyncConnection> 
     */
    static std::shared_ptr<AsyncConnection> Create(std::weak_ptr<bbt::network::libevent::IOThread> thread, OnErrCallback onerr_cb);
    ~AsyncConnection();

    /**
     * @brief 发起一个异步连接
     * 
     * @param ip 
     * @param port 
     * @param connect_timeout 连接建立超时时间
     * @param command_timeout 指令执行的超时时间
     * @param onconn_cb 连接建立成功的回调事件
     * @param onclose_cb 连接关闭时的回调事件
     * @return RedisErrOpt 
     */
    RedisErrOpt AsyncConnect(
        const std::string&  ip,
        short               port,
        int                 connect_timeout,
        int                 command_timeout,
        OnConnectCallback   onconn_cb,
        OnCloseCallback     onclose_cb);

    /* 执行异步指令 */
    RedisErrOpt AsyncExecCmd(std::string&& command, const OnReplyCallback& cb);

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
    explicit    AsyncConnection(std::weak_ptr<bbt::network::libevent::IOThread> thread, OnErrCallback onerr_cb);
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
    /* libevent 事件抛出函数 */
    void        OnEvent(short events);
    /* AsyncCommand操作 */
    bool        __PushAsyncCommand(AsyncCommand* async_cmd);
    std::vector<AsyncCommand*> __GetAsyncCommands();
    void        DestoryAllAsyncCommand();
    void        RegistWriteEvent();
    void        UnRegistWriteEvent();

    /* cfunc wapper */
    static void __CFuncOnConnect(const redisAsyncContext* ctx, int status);
    static void __CFuncOnClose(const redisAsyncContext* ctx, int status);
    static void __OnReply(redisAsyncContext* ctx, void* rpy, void* udata);

private:
    // std::shared_ptr<bbt::network::libevent::EventBase> m_io_ctx{nullptr};
    std::shared_ptr<bbt::network::libevent::IOThread>  m_io_thread{nullptr};
    AsyncConnectionPrivData     m_priv_data{nullptr};
    redisAsyncContext*          m_raw_async_ctx{nullptr};
    redisContextFuncs           m_io_funcs;
    bbt::net::IPAddress         m_redis_addr;
    redisOptions                m_redis_conn_option;
    struct timeval              m_connect_timeout;  // connect() 超时时间
    struct timeval              m_command_timeout;  // 执行指令，超时时间
    volatile bool               m_is_connected{false};

    bbt::thread::Queue<AsyncCommand*, 65535> 
                                m_lock_free_command_queue;
    std::shared_ptr<bbt::network::libevent::Event> 
                                m_write_event{nullptr};
    std::mutex                  m_write_event_mutex;
    std::atomic_bool            m_is_writing{false};

    OnConnectCallback           m_on_connect_handler{nullptr};
    IOWriteHandler              m_write_handler{nullptr};
    IOReadHandler               m_read_handler{nullptr};
    OnErrCallback               m_on_err_handler{nullptr};
    OnCloseCallback             m_on_close_handler{nullptr};

    const ConnId                m_conn_id;
    static ConnId               m_current_id;
};



} // namespace bbt::database::redis
