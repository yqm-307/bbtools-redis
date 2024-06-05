#include <bbt/redis/RedisClient.hpp>


namespace bbt::database::redis
{



void SetTimeval(struct timeval* tv, int time_ms)
{
    evutil_timerclear(tv);
    tv->tv_sec = time_ms / 1000;
    tv->tv_usec = (time_ms % 1000) / 1000;
}

RedisClient::RedisClient(const OnErrCallback& onerr_cb):
    m_on_error(onerr_cb)
{
    AssertWithInfo(onerr_cb != nullptr, "please check onerr_cb is right!");
}

RedisClient::~RedisClient()
{

}

RedisErrOpt RedisClient::AsyncConnect(
    bbt::network::libevent::Network& network,
    const std::string&  ip,
    short               port,
    int                 connect_timeout,
    int                 command_timeout,
    OnConnectCallback   onconn_cb,
    OnCloseCallback     onclose_cb)
{
    using namespace bbt::network::libevent;

    auto opt = std::make_shared<RedisOption>(network.GetAIOThread(), [this](const bbt::errcode::IErrcode& err){ this->OnError(dynamic_cast<const RedisErr&>(err)); });

    if (connect_timeout <= 0)
        return RedisErr{"connect timeout less then 0!", RedisErrType::Comm_ParamErr};

    if (command_timeout <= 0)
        return RedisErr{"command timeout less then 0!", RedisErrType::Comm_ParamErr};
    
    opt->SetTCP(ip.c_str(), port);
    opt->SetRedisOptions(REDIS_OPT_NONBLOCK);
    opt->SetRedisOptions(REDIS_OPT_REUSEADDR);
    opt->SetRedisOptions(REDIS_OPT_NOAUTOFREE);
    opt->SetRedisOptions(REDIS_OPT_NOAUTOFREEREPLIES);
    opt->SetConnectTimeout(connect_timeout);
    opt->SetCommandTimeout(command_timeout);    
    opt->SetOnConnect(onconn_cb);
    opt->SetOnClose(onclose_cb);
    
    AsyncContext* context = new AsyncContext(opt);

    return context->AsyncConnect();
}

void RedisClient::OnError(const RedisErr& err)
{
    // 初始化已完成检测
    m_on_error(err);
}

}
