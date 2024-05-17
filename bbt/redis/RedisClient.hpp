#pragma once
#include <bbt/redis/Define.hpp>
#include <bbt/redis/connect/AsyncConnection.hpp>

namespace bbt::database::redis
{

class RedisClient
{
public:
    RedisClient(const OnErrCallback& onerr_cb);
    ~RedisClient();
    RedisErrOpt AsyncConnect(
        bbt::network::libevent::Network& network,
        const std::string&  ip,
        short               port,
        int                 connect_timeout,
        int                 command_timeout,
        OnConnectCallback   onconn_cb,
        OnCloseCallback     onclose_cb
    );
protected:
    void Connect(RedisOption& opt);
    void OnError(const RedisErr& err);
private:
    OnErrCallback   m_on_error{nullptr};
};

}