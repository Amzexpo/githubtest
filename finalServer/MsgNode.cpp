#include "MsgNode.h"

//定义的接收结点，参数为传入的字符串长度， 以及消息id 调用了基类的构造函数，
RecvNode::RecvNode(short max_len, short msg_id):MsgNode(max_len),
_msg_id(msg_id){
}

//定义的发送结点，调用了基类的构造函数，要发送的信息包括头结点的长度，初始化过程中已经进行了字节序的转换
SendNode::SendNode(const char* msg, short max_len, short msg_id):MsgNode(max_len + HEAD_TOTAL_LEN)
, _msg_id(msg_id){
    //先发送id, 转为网络字节序
    short msg_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    memcpy(_data, &msg_id_host, HEAD_ID_LEN);
    //转为网络字节序
    short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(_data + HEAD_ID_LEN, &max_len_host, HEAD_DATA_LEN);
    memcpy(_data + HEAD_ID_LEN + HEAD_DATA_LEN, msg, max_len);
}