#pragma once
#include <iostream>
#include <array>
#include <cstring>
#include <memory>
#include <hiredis/adapters/libevent.h>
#include <bbt/base/net/IPAddress.hpp>
#include <bbt/base/thread/LockFreeQueue.hpp>
#include <bbt/network/adapter/libevent/EventBase.hpp>
#include <bbt/network/adapter/libevent/EventLoop.hpp>
#include <bbt/network/adapter/libevent/Event.hpp>
#include <bbt/network/adapter/libevent/Network.hpp>
#include "bbt/redis/Define.hpp"

namespace bbt::database::redis
{

class AsyncCommand
{
    friend class AsyncConnection;
public:
    AsyncCommand(std::weak_ptr<AsyncConnection> connection, std::string&& cmd, OnReplyCallback cb);
    ~AsyncCommand();

    void OnReply(RedisErrOpt err, std::shared_ptr<Reply> reply);
private:
    RedisErrOpt DoCommand(redisAsyncContext* context, redisCallbackFn* fn);
private:
    std::weak_ptr<AsyncConnection> m_connection;
    std::string     m_cmd;
    OnReplyCallback m_on_reply_handler{nullptr};
};

}