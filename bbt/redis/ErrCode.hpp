#pragma once
#include <bbt/base/errcode/Errcode.hpp>
#include <string>

namespace bbt::database::redis::err
{

enum RedisErrType
{
    ConnectionFailed            = 1,
    Comm_ParamErr               = 2,
    Comm_OOM                    = 3,
    Comm_TryAgain               = 4,
    Comm_UnDefErr               = 5,
    Reply_Err                   = 100,
    Reply_UnExpectedType        = 101,
    Reply_IsNil                 = 102,
};

class RedisErr:
    public bbt::errcode::Errcode<bbt::database::redis::err::RedisErrType>
{
public:
    RedisErr(const std::string& errinfo, RedisErrType errtype):Errcode(errinfo, errtype) {}
    ~RedisErr() {}

    virtual const std::string& What() const override { return GetMsg(); }
    virtual const bbt::database::redis::err::RedisErrType& Type() const override { return GetErrType(); }
    virtual const char* CWhat() const override { return What().c_str(); }
private:
};

} // namespace bbt::database::redis