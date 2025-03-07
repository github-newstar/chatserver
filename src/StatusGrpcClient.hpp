#include "const.h"
#include "Singleton.hpp"
#include "configMgr.hpp"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "GrpcPool.hpp"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;

class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;
public:
    ~StatusGrpcClient(){}
    GetChatServerRsp GetChatServer(const int uid);
    message::LoginRsp Login(int uid, std::string token);
private:
    StatusGrpcClient();
    std::unique_ptr<GrpcPool<StatusService, StatusService::Stub>> pool_;
};