#pragma once
#include <bbt/base/hash/BKDR.hpp>
#include <bbt/redis/Define.hpp>
#include <bbt/redis/connect/AsyncConnection.hpp>

namespace bbt::database::redis
{

struct Hash
{
    std::size_t operator()(const bbt::net::IPAddress& arg) const
    {
        return bbt::hash::BKDR::BKDRHash(arg.GetIPPort());
    }
};

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
    void OnError(const RedisErr& err);
private:
    std::unordered_map<bbt::net::IPAddress, RedisOption, Hash> m_redis_options_map;
    OnErrCallback   m_on_error{nullptr};
};

}