#pragma once
#include <bbt/network/adapter/libevent/LibeventConnection.hpp>
#include <bbt/redis/connect/AsyncCommand.hpp>
#include <bbt/redis/connect/RedisOption.hpp>



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
    public bbt::network::libevent::LibeventConnection,
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
    static std::shared_ptr<AsyncConnection> Create(AsyncContext* context);

    BBTATTR_FUNC_Ctor_Hidden explicit AsyncConnection(
        std::shared_ptr<bbt::network::libevent::IOThread> thread,
        AsyncContext*               context,
        int                         socket,
        const bbt::net::IPAddress&  addr);
    ~AsyncConnection();

    /* 执行异步指令 */
    RedisErrOpt AsyncExecCmd(std::string&& command, const OnReplyCallback& cb);

    void        Close();
    RedisErrOpt SetCommandTimeout(int timeout);
protected:

    void        OnError(const bbt::errcode::Errcode& err);
    /* libevent 事件抛出函数 */
    void        OnEvent(short events);
    /* AsyncCommand操作 */
    bool        __PushAsyncCommand(AsyncCommand* async_cmd);
    std::vector<AsyncCommand*> __GetAsyncCommands();
    void        DestoryAllAsyncCommand();
    void        RegistWriteEvent();
    void        UnRegistWriteEvent();

    /* cfunc wapper */
    static void __OnReply(redisAsyncContext* ctx, void* rpy, void* udata);

private:
    // std::shared_ptr<bbt::network::libevent::EventBase> m_io_ctx{nullptr};
    std::shared_ptr<bbt::network::libevent::IOThread>  m_io_thread{nullptr};
    redisContextFuncs           m_io_funcs;
    bbt::net::IPAddress         m_redis_addr;
    redisOptions                m_redis_conn_option;
    struct timeval              m_connect_timeout;  // connect() 超时时间
    struct timeval              m_command_timeout;  // 执行指令，超时时间
    volatile bool               m_is_connected{false};
    AsyncContext*               m_async_context{nullptr};

    bbt::thread::Queue<AsyncCommand*, 65535> 
                                m_lock_free_command_queue;
    std::shared_ptr<bbt::network::libevent::Event> 
                                m_write_event{nullptr};
    std::mutex                  m_write_event_mutex;
    std::atomic_bool            m_is_writing{false};

    IOWriteHandler              m_write_handler{nullptr};
    IOReadHandler               m_read_handler{nullptr};

    const ConnId                m_conn_id;
    static ConnId               m_current_id;
};



} // namespace bbt::database::redis
