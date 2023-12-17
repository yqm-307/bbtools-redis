#pragma once
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

    void ExecCmd();

    Reply* GetReply();

    bool IsConnected();

    void Disconnect();

protected:

private:
    redisContext* m_raw_ctx;
};

} // namespace bbt::db::redis