#pragma once
#include "bbt/redis/Define.hpp"

namespace bbt::database::redis
{



/**
 * @brief redis reply的简易封装
 * 
 */
class Reply
{
public:
    typedef std::shared_ptr<Reply> SPtr;
    Reply(redisReply*);
    ~Reply();

    operator bool() const;

    void Clear();


    template<typename OutParamType>
    RedisErrOpt Transform(OutParamType& value) const { return GetValue(value); }
protected:
    RedisErrOpt GetValue(int& value) const;
    RedisErrOpt GetValue(std::string& value) const;
    RedisErrOpt GetValue(double& value) const;

private:
    redisReply* m_reply{nullptr};
    ReplyType   m_type{ReplyType::kNone};
};




} // namespace bbt::database::redis