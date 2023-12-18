#pragma once
#include <bbt/net/IPAddress.hpp>
#include "Define.hpp"
#include "reply/Reply.hpp"

namespace bbt::database::redis
{

class Connection
{
public:
    /**
     * @brief 通过ip、端口连接到redis
     * 
     * @param ip 
     * @param port 
     */
    Connection(const std::string& ip="localhost", short port=6379);
    ~Connection();

    /**
     * @brief 重新连接到redis。如果连接已经断开了，则执行重连；否则什么都不做
     */
    RedisErrOpt ReConnect();

    std::pair<RedisErrOpt, Reply::SPtr> SyncExecCmd(std::vector<std::string> command);
    std::pair<RedisErrOpt, Reply::SPtr> SyncExecCmd(const std::string& command);
    std::pair<RedisErrOpt, Reply::SPtr> SyncExecCmd(const char*, ...);

    Reply* GetReply();

    bool IsConnected();

    void Disconnect();

protected:
    RedisErrOpt Connect();
    std::pair<RedisErrOpt, std::shared_ptr<Reply>> CheckReply(redisReply* reply);

private:
    redisContext* m_raw_ctx;
    bbt::net::IPAddress m_redis_address;
    static std::mutex m_buf_lock;
    static char m_format_buffer[COMMAND_MAX_SIZE];
};

#define RedisFormatCommand(target, format, ...) redisFormatCommand(target, format, ##__VA_ARGS__)

} // namespace bbt::db::redis