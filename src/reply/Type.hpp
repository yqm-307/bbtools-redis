#pragma once
#include <bbt/type/type_traits.hpp>
#include "Define.hpp"

namespace bbt::database::redis::detail
{

template<typename ParamType>
struct __ToReplyTypeEnum {
    const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kNone;
};

template<>
struct __ToReplyTypeEnum<int> {
    const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kInteger;
};

template<>
struct __ToReplyTypeEnum<double> {
    const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kDouble;
};

template<>
struct __ToReplyTypeEnum<std::string> {
    const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kCString;
};

template<>
struct __ToReplyTypeEnum<char*> {
    const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kCString;
};

template<>
struct __ToReplyTypeEnum<bool> {
    const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kBool;
};

template<>
struct __ToReplyTypeEnum<bbt::database::redis::Nil> {
    const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kInteger;
};

template<>
struct __ToReplyTypeEnum<int64_t> {
    const bbt::database::redis::ReplyType type = bbt::database::redis::ReplyType::kBignum;
};


template<typename ParamType>
using ToReplyType = typename __ToReplyTypeEnum<ParamType>;

template<typename Type>
static inline bbt::database::redis::ReplyType GetReplyType()
{
    return ToReplyType<Type>::type;
}

} // namespace bbt::database::redis