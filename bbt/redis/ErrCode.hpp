#pragma once
#include <bbt/base/errcode/Errcode.hpp>
#include <string>

namespace bbt::database::redis::err
{

enum RedisErrType
{
    OK                          = 0,
    ConnectionFailed            = 1,
    Comm_ParamErr               = 2,
    Comm_OOM                    = 3,
    Comm_TryAgain               = 4,
    Comm_UnDefErr               = 5,
    Reply_Err                   = 100,
    Reply_UnExpectedType        = 101,
    Reply_IsNil                 = 102,
    Hiredis_Default             = 201,  // hiredis api 导致的错误
};

class RedisErr:
    public bbt::errcode::Errcode
{
public:
    RedisErr(const std::string& errinfo, RedisErrType errtype):Errcode(errinfo, errtype) {}
    ~RedisErr() {}

    virtual bool IsErr() const override { return (Type() != OK); };
private:
};

} // namespace bbt::database::redis