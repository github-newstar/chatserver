#pragma once
#include "data.hpp"
#include "Singleton.hpp"
#include <queue>
#include<functional>
#include <map>
#include<string>
#include <condition_variable>
#include <memory>
#include<thread>

class CSession;
class LogicNode;
using FunCallback = std::function<void (std::shared_ptr<CSession>, 
                        const short &msg_id, const std::string &msg_data)>; 
class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;
    friend class TestableLogicSystem;
public:
    ~LogicSystem();
    void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
    LogicSystem();
    void DealMsg();
    void RegisterCallback();
    void LoginHandler(std::shared_ptr<CSession> session, const short &msg_id, const std::string &msg_data);
    std::thread  workThread_;
    std::queue<std::shared_ptr<LogicNode>> msgQue_;
    std::mutex mtx_;
    std::condition_variable consume_;;
    
    bool b_stop_;
    std::map<short, FunCallback> fun_callbacks_;
    std::unordered_map<int, std::shared_ptr<UserInfo>> users_;

    
};