#include "CSession.hpp"
#include "CServer.hpp"
#include "MsgNode.hpp"
#include "src/const.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include "LogicSystem.hpp"


CSession::CSession(boost::asio::io_context  &ioc, CServer *server) :
    socket_(ioc) , server_(server) , b_close_(false) , b_head_prase_(false){
        boost::uuids::uuid  a_uuid = boost::uuids::random_generator()();
        uuid_ = boost::uuids::to_string(a_uuid);
        recv_head_node_ = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

CSession::~CSession(){
    std::cout <<  " csession destruct " << std::endl;
}

tcp::socket& CSession::GetSocket() {
	return socket_;
}

std::string& CSession::GetUuid() {
	return uuid_;
}

void CSession::Start(){
    AsyncReadHead(HEAD_TOTAL_LEN);
}
void CSession::Send(char *msg, short max_len, short msgid){
    std::unique_lock<std::mutex> lock(send_mtx_);
	int send_que_size = sendQue_.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " <<  uuid_<< " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	sendQue_.push(std::make_shared<SendNode>(msg, max_len, msgid));
	if (send_que_size>0) {
		return;
	}
	auto& msgnode = sendQue_.front();
    lock.unlock();
	boost::asio::async_write(socket_, boost::asio::buffer(msgnode->data_, msgnode->totalLen_), 
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf())); 
}
void CSession::Send(std::string msg, short msgid){
    Send(const_cast<char*>(msg.c_str()), msg.size(), msgid);
}
void CSession::Close(){
    socket_.close();
    b_close_ = true;
}
std::shared_ptr<CSession> CSession::SharedSelf(){
    return shared_from_this();
}
void CSession::AsyncReadBody(int total_len){
    auto self = shared_from_this();
    asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code &ec, 
    std::size_t bytesTransfered){
        try{
            if(ec){
                std::cout << "handle read failed , error is " << ec.message() << std::endl;
                Close();
                server_->ClearSession(uuid_);
                return;
            }
            
            if(bytesTransfered <total_len){
                std::cout << "read length not match, read "<<bytesTransfered <<
                ", but total is " <<  total_len << std::endl;
                Close();
                server_->ClearSession(uuid_);
                return;
            }
            memcpy(recv_msg_node_->data_, data_, bytesTransfered);
            recv_msg_node_->curLen_ += bytesTransfered;
            recv_head_node_->data_[recv_msg_node_->curLen_] = '\0';
			std::cout << "receive data is " << recv_msg_node_->data_ << std::endl;

			LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), recv_msg_node_));
            //继续
			AsyncReadHead(HEAD_TOTAL_LEN);
        }
        catch (std::exception &e){
            std::cerr << "Excetion code is " <<  e.what() << std::endl;
        }
    });
}

void CSession::AsyncReadHead(int total_len){
    auto self = shared_from_this();
    
    asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, 
    std::size_t bytesTransfered){
            try{
                if(ec){
                    std::cerr<<"handler read error, error is  " << ec.message() << std::endl;
                    server_->ClearSession(uuid_);
                    Close();
                    return;
                }
                if(bytesTransfered < HEAD_TOTAL_LEN){
                    std::cout << " read length not match, read "<<bytesTransfered <<
                    ", but total is " <<  HEAD_TOTAL_LEN << std::endl;
                    server_->ClearSession(uuid_);
                    Close();
                    return;
                }
                recv_head_node_->Clear();
                ::memcpy(recv_head_node_->data_, data_, bytesTransfered);
                
                
                //解析数据
                short msg_id = 0;
                memcpy(&msg_id, recv_head_node_->data_, HEAD_ID_LEN);
                msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
                std::cout << "msg_id is " << msg_id << std::endl;
                //错误检查
                if (msg_id > MAX_LEN) {
                    std::cout << "invalid msg_id is " << msg_id << std::endl;
                    server_->ClearSession(uuid_);
                    return;
                }
                short msg_len = 0;
                memcpy(&msg_len, recv_head_node_->data_ + HEAD_ID_LEN, HEAD_DATA_LEN);
                //
                //读长度做校验
                msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
                std::cout << "msg_len is " << msg_len << std::endl;
                if (msg_len > MAX_LEN) {
                    std::cout << "invalid data length is " << msg_len << std::endl;
                    server_->ClearSession(uuid_);
                    return;
                }
                recv_msg_node_ = std::make_shared<RecvNode>(msg_len, msg_id);
                AsyncReadBody(msg_len);
            }
            catch(std::exception& e){
                std::cerr << "excetoion code is "<< e.what() <<std::endl;
            }
    });
}

void CSession::HandleWrite(const boost::system::error_code &ec, std::shared_ptr<CSession> shared_self){
    try{
        if(!ec){
            std::unique_lock<std::mutex> lock(send_mtx_);
            if(!sendQue_.empty()){
                auto &msgNode = sendQue_.front();
                sendQue_.pop();
                lock.unlock();
                boost::asio::async_write(socket_, boost::asio::buffer(msgNode->data_, msgNode->totalLen_),
            std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));
            }
        }else{
            std::cout << "handle write failed " << ec.message() << std::endl;
            Close();
            server_->ClearSession(uuid_);
        }
    }
    catch (std::exception &e){
        std::cerr << "Excetion code is " << e.what() << std::endl;
    }
}

void CSession::asyncReadFull(
    std::size_t maxlen,
    std::function<void(const boost::system::error_code &, std::size_t)>
        handler){
    memset(data_, 0, MAX_LEN);
    asyncReadLen(0, maxlen, handler);
        }

void CSession::asyncReadLen(std::size_t read_len , std::size_t total_len,
     std::function<void(const boost::system::error_code&, std::size_t )> handler){
    auto self = shared_from_this();
    socket_.async_read_some(
        boost::asio::buffer(data_ + read_len, total_len - read_len),
        [read_len, total_len, handler, self](
            const boost::system::error_code &ec, std::size_t bytesTransfered) {
            if (ec) {
                handler(ec, read_len + bytesTransfered);
                return;
            }
            // TODO 不懂
            if (read_len + bytesTransfered >= total_len) {
                handler(ec, read_len + bytesTransfered);
                return;
            }
            
            self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
        });
     }

LogicNode::LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recvNode) :
session_(session) , recvNode_(recvNode){}