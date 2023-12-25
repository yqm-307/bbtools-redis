#include "connect/Connect.hpp"

int main()
{
    /* 1、建立redis连接 */
    auto* conn = new bbt::database::redis::Connection;
    auto conn_err = conn->ReConnect();
    
    if (conn_err != std::nullopt) {
        printf("%s\n", conn_err.value().What().c_str());
        return -1;
    }

    /* 2、执行 set 操作 */
    auto [err1, reply1] = conn->SyncExecCmd("Set name XiaoMing");
    if (err1 != std::nullopt) {
        printf("%s\n", err1.value().What().c_str());
        return -1;
    } else {
        printf("Do cmd: Set name XiaoMing\n");
    }

    /* 3、执行 get 操作，并打印结果  */
    auto [err2, reply2] = conn->SyncExecCmd("Get name");
    if (err2 != std::nullopt) {
        printf("%s\n", err2.value().What().c_str());
        return -1;
    } else {
        printf("Do cmd: Get name\n");
        std::string value;
        auto err3 = reply2->Transform(value);
        if (err3 != std::nullopt) {
            printf("Transform failed! %s\n", err3.value().What().c_str());
            return -1;
        }
        printf("result: %s\n", value.c_str());
    }

    return 0;
}