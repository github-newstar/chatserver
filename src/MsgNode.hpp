#pragma  once
#include <boost/asio/detail/socket_ops.hpp>
#include "const.h"

class MsgNode 
{
public:
    MsgNode(short maxlen) : curLen_(maxlen) , totalLen_(maxlen +  1){
        data_ = new char[totalLen_  + 1];
        data_[totalLen_] = '\0';
    }
    
    void Clear(){
        ::memset(data_, 0,  totalLen_);
        curLen_ = 0;
    }

    char * data_;
    short curLen_;
    short totalLen_;
};

class RecvNode : public MsgNode{
    friend class LogicNode;
public:
    RecvNode(short maxlen, short msgId) : MsgNode(maxlen) , msgId_(msgId){}
    short msgId_;
};

class SendNode : public MsgNode{
public:
    SendNode(char *msg, short max_len , short msgId) : MsgNode(max_len + HEAD_TOTAL_LEN), msgId_(msgId){
        //拷贝id
        short msg_id = boost::asio::detail::socket_ops::host_to_network_short(msgId);
        std::memcpy(data_, &msg_id, HEAD_ID_LEN);
        //拷贝长度和数据
        short msg_len = boost::asio::detail::socket_ops::host_to_network_short(max_len);
        std::memcpy(data_ + HEAD_ID_LEN, &msg_len, HEAD_DATA_LEN);
        std::memcpy(data_ + HEAD_TOTAL_LEN, msg, max_len);

    }
private:
    short msgId_;
};