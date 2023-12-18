#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include <thread>
#include <cmath>
#include <optional>
#include <memory>
#include <hiredis/hiredis.h>
#include "ErrCode.hpp"

namespace bbt::database::redis
{

//XXX 目前使用固定的缓存存数据，后续可以考虑大数据直接分批一劳永逸的解决问题
explicit const inline size_t COMMAND_MAX_SIZE = 1024 * 1024 * 8;

class Reply;
class Connection;
typedef err::RedisErr RedisErr;
typedef std::optional<err::RedisErr> RedisErrOpt;
typedef err::RedisErrType RedisErrType;

struct Nil {};

enum ReplyType
{
    kNone =      0,
    kCString =   REDIS_REPLY_STRING,
    kArray =     REDIS_REPLY_ARRAY,
    kInteger =   REDIS_REPLY_INTEGER,
    kNil =       REDIS_REPLY_NIL,
    kStatus =    REDIS_REPLY_STATUS,
    kError =     REDIS_REPLY_ERROR,
    kDouble =    REDIS_REPLY_DOUBLE,
    kBool =      REDIS_REPLY_BOOL,
    kMap =       REDIS_REPLY_MAP,
    kSet =       REDIS_REPLY_SET,
    kAttr =      REDIS_REPLY_ATTR,
    kPush =      REDIS_REPLY_PUSH,
    kBignum =    REDIS_REPLY_BIGNUM,
    kVerb =      REDIS_REPLY_VERB,
};

}