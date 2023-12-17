#pragma once
#include <bbt/errcode/Errcode.hpp>

namespace bbt::database::redis
{

enum ErrType
{

};

class RedisErr:
    public bbt::errcode::Errcode<ErrType>
{
public:

private:
};

} // namespace bbt::database::redis