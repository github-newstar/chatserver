#pragma  once

#include <memory>
#include "const.h"
// #include "src/CServer.hpp"
#include <boost/asio.hpp>
#include <mutex>
#include <queue>
#include "MsgNode.hpp"

using tcp = boost::asio::ip::tcp;
class CServer;
class LogicSystem;


class CSession : public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context &ioc, CServer *server);
	~CSession();
	tcp::socket& GetSocket();
	std::string& GetUuid();

	void Start();
	void Send(char *msg, short max_len, short msgid);
	void Send(std::string msg, short msgid);
	void Close();

	std::shared_ptr<CSession> SharedSelf();
	void AsyncReadBody(int length);
	void AsyncReadHead(int total_len);
private:
	void asyncReadFull(std::size_t maxlen, std::function<void(const boost::system::error_code&, std::size_t )> handler);
	void asyncReadLen(std::size_t read_len , std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t )> handler);
	void HandleWrite(const boost::system::error_code &ec, std::shared_ptr<CSession> shared_self);
	
	tcp::socket socket_;
	std::string uuid_;
	char data_[MAX_LEN];
	CServer* server_;
	bool b_close_;
	std::queue<std::shared_ptr<SendNode>> sendQue_;
	std::mutex send_mtx_;
	std::shared_ptr<RecvNode> recv_msg_node_;

	bool b_head_prase_;
	std::shared_ptr<MsgNode> recv_head_node_;


};

class LogicNode{
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);
private:
	std::shared_ptr<CSession> session_;
	std::shared_ptr<RecvNode> recvNode_;
};