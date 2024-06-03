#include <cstring>
#include <hiredis/async.h>
#include <bbt/redis/connect/AsyncConnection.hpp>
#include <bbt/redis/reply/Reply.hpp>
#include <bbt/redis/connect/AsyncContext.hpp>

namespace bbt::database::redis
{

ConnId AsyncConnection::m_current_id = 1;

std::shared_ptr<AsyncConnection> AsyncConnection::Create(AsyncContext* context)
{
    auto thread = context->GetBindThread();
    if (thread == nullptr)
        return nullptr;

    int socket = context->GetSocketFd();
    if (socket < 0)
        return nullptr;
    
    auto peer_addr = context->GetPeerAddress();

    return std::make_shared<AsyncConnection>(thread, context, socket, peer_addr);
}

void AsyncConnection::__OnReply(redisAsyncContext* ctx, void* rpy, void* udata)
{
    /**
     * hiredis 传入的reply由Reply接管，Reply对象释放时释放内存，并使用shared_ptr管理Reply对象
     */
    auto* async_command = static_cast<AsyncCommand*>(udata);
    auto* reply = static_cast<redisReply*>(rpy);

    if (ctx->err) {
        async_command->OnReply(RedisErr(ctx->errstr, RedisErrType::Reply_Err), nullptr);
    } else {
        auto reply_sptr = (reply == nullptr) ? nullptr : std::make_shared<Reply>(reply);
        async_command->OnReply(std::nullopt, reply_sptr);
    }

    /* 回调执行完毕，释放对象 */
    delete async_command;
}


AsyncConnection::AsyncConnection(std::shared_ptr<bbt::network::libevent::IOThread> thread,
                                 AsyncContext* context,
                                 int socket,
                                 const bbt::net::IPAddress& addr):
    bbt::network::libevent::LibeventConnection(thread, socket, addr),
    m_io_thread(thread),
    m_conn_id(m_current_id++),
    m_async_context(context)
{
}

AsyncConnection::~AsyncConnection()
{
    // 如果已经关闭了，就不需要走释放流程了
    if (IsConnected())
        Close();

    UnRegistWriteEvent();
    delete m_async_context;
}

void AsyncConnection::DestoryAllAsyncCommand()
{
    auto commands = __GetAsyncCommands();
    for (auto&& command_ptr : commands)
        delete command_ptr;
}

void AsyncConnection::Close()
{
    m_async_context->Close();
    SetStatus(bbt::network::ConnStatus::DECONNECTED);
}


RedisErrOpt AsyncConnection::AsyncExecCmd(std::string&& command, const OnReplyCallback& cb)
{
    if (command.empty())
        return RedisErr("command is empty", RedisErrType::Comm_ParamErr);

    if (!__PushAsyncCommand(new AsyncCommand(weak_from_this(), std::move(command), cb)))
        return RedisErr("push command failed! please try again!", RedisErrType::Comm_TryAgain);
    
    RegistWriteEvent();

    return std::nullopt;
}

RedisErrOpt AsyncConnection::SetCommandTimeout(int timeout)
{
    return m_async_context->SetCommandTimeout(timeout);
}

void AsyncConnection::OnError(const bbt::errcode::Errcode& err)
{
    m_async_context->OnError(dynamic_cast<const RedisErr&>(err));
}

bool AsyncConnection::__PushAsyncCommand(AsyncCommand* async_cmd)
{
    return m_lock_free_command_queue.Push(async_cmd);
}

std::vector<AsyncCommand*> AsyncConnection::__GetAsyncCommands()
{
    std::vector<AsyncCommand*> async_commands;

    do {
        async_commands.push_back(nullptr);
    } while(m_lock_free_command_queue.Pop(async_commands.back()));
    async_commands.pop_back();

    return async_commands;
}

void AsyncConnection::OnEvent(short events)
{
    auto commands = __GetAsyncCommands();
    UnRegistWriteEvent();

    for (auto&& command : commands) {
        auto err = command->DoCommand(m_async_context, AsyncConnection::__OnReply);
        if (err != std::nullopt)
            OnError(err.value());
    }
}

void AsyncConnection::RegistWriteEvent()
{
    using namespace bbt::network::libevent;
    bool is_writing = false;
    /* 如果 m_is_writing 是 false，则修改为true，并发挥true；否则返回false，不做修改 */
    if (!m_is_writing.compare_exchange_strong(is_writing, true))
        return;

    //XXX 观察后续去除
    AssertWithInfo(m_write_event == nullptr, "multi thread order problem!");
    if (m_write_event != nullptr)
        return;
    
    auto weak_ptr = weak_from_this();
    Assert(!weak_ptr.expired());
    m_write_event = m_io_thread->RegisterEvent(-1, EventOpt::PERSIST, [weak_ptr](std::shared_ptr<Event>, short events){
        auto pthis = weak_ptr.lock();
        if (!pthis) return;
        pthis->OnEvent(events);
    });

    auto err = m_write_event->StartListen(5);
    if (err.IsErr())
        OnError(RedisErr("listen event failed!", RedisErrType::Comm_UnDefErr));
}

void AsyncConnection::UnRegistWriteEvent()
{
    if (m_is_writing)
        return;

    if (m_write_event)
        m_write_event->CancelListen();

    m_write_event = nullptr;

    m_is_writing.exchange(false);
}

} // namespace bbt::database::redis
