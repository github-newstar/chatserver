#include "StatusGrpcClient.hpp"
#include "message.pb.h"

message::GetChatServerRsp StatusGrpcClient::GetChatServer(const int uid) {
    ClientContext context;
    GetChatServerRsp reply;
    GetChatServerReq request;
    request.set_uid(uid);

    auto stub = pool_->getConnection();
    Status status = stub->GetChatServer(&context, request, &reply);
    
    //defer处理连接归还
    Defer defer([&stub, this]() { pool_->returnConnection(std::move(stub)); });

    if (status.ok()) {
        return reply;
    } else {
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}
StatusGrpcClient::StatusGrpcClient() {
    auto &gCfgMgr = ConfigMgr::GetInstance();
    std::string host = gCfgMgr["StatusServer"]["Host"];
    std::string port = gCfgMgr["StatusServer"]["Port"];
    pool_.reset(
        new GrpcPool<StatusService, StatusService::Stub>(5, host, port));
}

message::LoginRsp StatusGrpcClient::Login(int uid, std::string token){
    ClientContext context;
    message::LoginRsp reply;
    message::LoginReq request;
    request.set_uid(uid);
    request.set_token(token);
    
    auto stub = pool_->getConnection();
    Status status = stub->Login(&context, request, &reply);
    Defer defer([&stub, this](){
            pool_->returnConnection(std::move(stub));
    });
    if(status.ok()){
        return reply;
    }
    else{
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}