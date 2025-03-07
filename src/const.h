#pragma once
#include <functional>

                                  // 
enum ErrorCodes{
    Success = 0,
    ErrorJson = 1001,
    RPCFailed = 1002,
    VarifyExpired = 1003, // 验证码已过期
    VarifyErr = 1004, // 验证码错误
    UserExist = 1005, // 用户已存在
    PasswdErr = 1006, // 密码错误
    EmailNotMatch = 1007, // 邮箱不匹配
    PasswdUpFailed = 1008, // 密码更新失败
    PasswdInvalid = 1009, // 密码无效
    TokenInvalid = 1010, // token无效
    UidInvalid = 1011, // uid无效
};

class ConfigMgr;
extern ConfigMgr gCfgMgr;
class Defer{
public:
    //接受一个lambda表达式或者函数指针
    Defer(std::function<void()> func) : func_(func){}
    
    //析构函数中执行传入的函数
    ~Defer(){
        func_();
    }
private:
    std::function<void()> func_;
};

#define MAX_LEN 2048
#define HEAD_TOTAL_LEN 4
#define HEAD_ID_LEN 2
#define HEAD_DATA_LEN 2
#define MAX_RECVQUE 1000
#define MAX_SENDQUE 1000

enum MSG_IDS{
    MSG_CHAT_LOGIN = 1005,
    MSG_CHAT_LOGIN_RSP = 1006,
};