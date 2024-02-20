#pragma once
#include <bbt/base/type/type_traits.hpp>
#include "Define.hpp"

namespace bbt::database::redis::detail
{

template<typename ParamType>
struct __ToReplyTypeEnum {
    static const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kNone;
};

template<>
struct __ToReplyTypeEnum<int> {
    static const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kInteger;
};

template<>
struct __ToReplyTypeEnum<double> {
    static const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kDouble;
};

template<>
struct __ToReplyTypeEnum<std::string> {
    static const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kCString;
};

template<>
struct __ToReplyTypeEnum<char*> {
    static const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kCString;
};

template<>
struct __ToReplyTypeEnum<bool> {
    static const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kBool;
};

template<>
struct __ToReplyTypeEnum<bbt::database::redis::Nil> {
    static const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kInteger;
};

template<>
struct __ToReplyTypeEnum<int64_t> {
    static const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kBignum;
};


template<typename ParamType>
using ToReplyType = __ToReplyTypeEnum<ParamType>;

template<typename Type>
static inline bbt::database::redis::ReplyType GetReplyType()
{
    return ToReplyType<Type>::type;
}

} // namespace bbt::database::redis