#include <cstring>
#include <hiredis/async.h>
#include "bbt/redis/connect/AsyncConnection.hpp"
#include "bbt/redis/reply/Reply.hpp"

namespace bbt::database::redis
{

ConnId AsyncConnection::m_current_id = 1;

std::shared_ptr<AsyncConnection> AsyncConnection::Create(std::weak_ptr<bbt::network::libevent::IOThread> thread, OnErrCallback onerr_cb)
{
    return std::shared_ptr<AsyncConnection>(new AsyncConnection(thread, onerr_cb));
}


void SetTimeval(struct timeval* tv, int time_ms)
{
    evutil_timerclear(tv);
    tv->tv_sec = time_ms / 1000;
    tv->tv_usec = (time_ms % 1000) / 1000;
}

void AsyncConnection::__CFuncOnConnect(const redisAsyncContext* ctx, int status)
{
    if (ctx->c.privdata == nullptr)
        return;

    auto priv = static_cast<AsyncConnectionPrivData*>(ctx->c.privdata);
    if (status == REDIS_OK)
        priv->m_on_connect(std::nullopt);
    else
        priv->m_on_connect(RedisErr(ctx->errstr, RedisErrType::ConnectionFailed));
}

void AsyncConnection::__CFuncOnClose(const redisAsyncContext* ctx, int status)
{
    if (ctx->c.privdata == nullptr)
        return;

    auto priv = static_cast<AsyncConnectionPrivData*>(ctx->c.privdata);
    if (status == REDIS_OK)
        priv->m_on_close(std::nullopt);
    else
        priv->m_on_close(RedisErr(ctx->errstr, RedisErrType::Comm_ParamErr));
}

void AsyncConnection::__OnReply(redisAsyncContext* ctx, void* rpy, void* udata)
{
    /*
        reply 由hiredis申请，由本库负责释放
        udata 也由本库负责释放
    */
    auto* context = static_cast<CommandContext*>(udata);
    auto* reply = static_cast<redisReply*>(rpy);

    auto conn_ptr = context->redis_connection.lock();
    if (conn_ptr != nullptr && context->on_reply_handler) {
        if (ctx->err) {
            context->on_reply_handler(RedisErr(ctx->errstr, RedisErrType::Reply_Err), nullptr);
        } else {
            auto reply_sptr = (reply == nullptr) ? nullptr : std::make_shared<Reply>(reply);
            context->on_reply_handler(std::nullopt, reply_sptr);
        }
    }

    /* 执行后，释放掉我们执行操作时申请的udata */
    delete context;
}


AsyncConnection::AsyncConnection(std::weak_ptr<bbt::network::libevent::IOThread> thread, OnErrCallback onerr_cb):
    m_io_thread(thread),
    m_conn_id(m_current_id++),
    m_on_err_handler(onerr_cb)
{
}

AsyncConnection::~AsyncConnection()
{
    Disconnect();
}

RedisErrOpt AsyncConnection::AsyncConnect(
    const std::string&  ip,
    short               port,
    int                 connect_timeout,
    int                 command_timeout,
    OnConnectCallback   onconn_cb,
    OnCloseCallback     onclose_cb)
{
    using namespace bbt::network::libevent;
    m_on_connect_handler = onconn_cb;
    m_on_close_handler = onclose_cb;

    if (connect_timeout <= 0)
        return RedisErr("connect timeout less then 0!", RedisErrType::Comm_ParamErr);

    if (command_timeout <= 0)
        return RedisErr("command timeout less than 0!", RedisErrType::Comm_ParamErr);

    memset(&m_redis_conn_option, '\0', sizeof(m_redis_conn_option));
    REDIS_OPTIONS_SET_TCP(&m_redis_conn_option, ip.c_str(), port);
    m_redis_conn_option.options |= REDIS_OPT_NONBLOCK;
    m_redis_conn_option.options |= REDIS_OPT_REUSEADDR;
    m_redis_conn_option.options |= REDIS_OPT_NOAUTOFREE;
    m_redis_conn_option.options |= REDIS_OPT_NOAUTOFREEREPLIES;

    SetTimeval(&m_connect_timeout, connect_timeout);
    m_redis_conn_option.command_timeout = &m_command_timeout;
    SetTimeval(&m_command_timeout, command_timeout);
    m_redis_conn_option.connect_timeout = &m_connect_timeout;

    __InitPrivData();

    return Connect();
}

RedisErrOpt AsyncConnection::Reconnect()
{
    if (IsConnected())
        return std::nullopt;
    
    return Connect();
}

RedisErrOpt AsyncConnection::Connect()
{
    m_raw_async_ctx = redisAsyncConnectWithOptions(&m_redis_conn_option);
    if (m_raw_async_ctx == nullptr) {
        return RedisErr("new connection failed!", RedisErrType::ConnectionFailed);
    } else if (m_raw_async_ctx->err != 0) {
        return RedisErr(m_raw_async_ctx->errstr, RedisErrType::ConnectionFailed);
    }
    
    event_base* base = m_io_thread->GetEventLoop()->GetEventBase()->m_io_context;
    if (redisLibeventAttach(m_raw_async_ctx, base) != REDIS_OK)
        return RedisErr(m_raw_async_ctx->errstr, RedisErrType::ConnectionFailed);

    if (redisAsyncSetConnectCallback(m_raw_async_ctx, __CFuncOnConnect) != REDIS_OK)
        return RedisErr(m_raw_async_ctx->errstr, RedisErrType::ConnectionFailed);

    if (redisAsyncSetDisconnectCallback(m_raw_async_ctx, __CFuncOnClose) != REDIS_OK)
        return RedisErr(m_raw_async_ctx->errstr, RedisErrType::ConnectionFailed);

    return std::nullopt;
}

void AsyncConnection::__InitPrivData()
{
    auto weak_this = weak_from_this();

    m_priv_data.m_on_connect = [weak_this](RedisErrOpt err){
        auto pthis = weak_this.lock();
        if (!pthis) return;
        pthis->OnConnect(err);
    };

    m_priv_data.m_on_close = [weak_this](RedisErrOpt err){
        auto pthis = weak_this.lock();
        if (!pthis) return;
        pthis->OnClose(err);
    };

    REDIS_OPTIONS_SET_PRIVDATA(&m_redis_conn_option, &m_priv_data, NULL);
}


RedisErrOpt AsyncConnection::Disconnect()
{
    if (!IsConnected())
        return std::nullopt;

    if (m_event)
        m_event->CancelListen();

    redisAsyncDisconnect(m_raw_async_ctx);
    redisAsyncFree(m_raw_async_ctx);
    m_raw_async_ctx = nullptr;
    return std::nullopt;
}

bool AsyncConnection::IsConnected() const
{
    return m_is_connected;
}

void AsyncConnection::Close()
{
    redisAsyncDisconnect(m_raw_async_ctx);
}


RedisErrOpt AsyncConnection::AsyncExecCmd(const std::string& command, const OnReplyCallback& cb)
{
    if (command.empty())
        return RedisErr("command is empty", RedisErrType::Comm_ParamErr);

    if (!__PushAsyncCommand(std::make_unique<AsyncCommand>(weak_from_this(), cb)))
        return RedisErr("push command failed! please try again!", RedisErrType::Comm_TryAgain); 

    return std::nullopt;
}

redisFD AsyncConnection::GetSocket() const
{
    if (IsConnected()) {
        return m_raw_async_ctx->c.fd;
    }

    return REDIS_INVALID_FD;
}

ConnId AsyncConnection::GetConnId() const
{
    return m_conn_id;
}


RedisErrOpt AsyncConnection::SetCommandTimeout(int timeout)
{
    if (timeout <= 0)
        return RedisErr("timeout less then 0!", RedisErrType::Comm_ParamErr);
    
    SetTimeval(&m_command_timeout, timeout);
    if (redisAsyncSetTimeout(m_raw_async_ctx, m_command_timeout) != REDIS_OK) {
        return RedisErr(m_raw_async_ctx->errstr, RedisErrType::Comm_OOM);
    }

    return std::nullopt;
}

void AsyncConnection::OnConnect(RedisErrOpt err)
{
    using namespace bbt::network::libevent;

    m_is_connected = true;
    if (m_on_connect_handler == nullptr) {
        OnError(RedisErr("no set on connect handle!", RedisErrType::Comm_ParamErr));
        return;
    }

    m_on_connect_handler(err, this);

    auto weak_ptr = weak_from_this();
    m_event = m_io_thread->RegisterEvent(-1, EventOpt::PERSIST, [weak_ptr](std::shared_ptr<Event>, short events){
        auto pthis = weak_ptr.lock();
        if (!pthis) return;
        pthis->OnEvent(events);
    });

}

void AsyncConnection::OnError(RedisErrOpt err)
{
    AssertWithInfo(m_on_err_handler != nullptr, "please set onerror handle!");
    m_on_err_handler(err);
}

void AsyncConnection::OnClose(RedisErrOpt err)
{
    m_is_connected = false;
    if (m_on_close_handler == nullptr) {
        OnError(RedisErr("no set on close handle!", RedisErrType::Comm_ParamErr));
        return;
    }

    m_on_close_handler(err, this);
}

bool AsyncConnection::__PushAsyncCommand(std::unique_ptr<AsyncCommand>&& async_cmd)
{
    return m_lock_free_command_queue.Push(async_cmd);
}

std::vector<std::unique_ptr<AsyncCommand>> AsyncConnection::__GetAsyncCommands()
{
    std::vector<std::unique_ptr<AsyncCommand>> async_commands;

    do {
        async_commands.push_back(nullptr);
    } while(m_lock_free_command_queue.Pop(async_commands.back()));
    async_commands.pop_back();

    return async_commands;
}

void AsyncConnection::OnEvent(short events)
{
}


} // namespace bbt::database::redis
