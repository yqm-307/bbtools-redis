#include "Connect.hpp"

namespace bbt::database::redis
{

Connection::Connection(const std::string& ip, short port)
{
}

Connection::~Connection()
{
    redisFree(m_raw_ctx);
}

}// namespace bbt::database::redis