#ifndef _MsgNode_H_
#define _MsgNode_H_

#include <iostream>
#include <stdlib.h>
#include <boost/asio.hpp>

#include "Server.h"
#include "Session.h"
#include "const.h"

class MsgNode{
    friend class Session;
public:
    MsgNode(char* msg, short max_len) : _total_len(max_len + HEAD_LENGTH), _cur_len(0) {
        _data = new char[max_len + 1];
        memcpy(_data, &max_len, HEAD_LENGTH);
        memcpy(_data+ HEAD_LENGTH, msg, max_len);
        _data[_total_len] = '\0';
    }

     MsgNode(short max_len):_total_len(max_len),_cur_len(0) {
        _data = new char[_total_len +1]();
    }

//避免多次结点构造所产生的开销
    ~MsgNode() {
       ::memset(_data, 0, _total_len);
        _cur_len = 0;
    }

    void Clear() {
        std::memset(_data, 0, _total_len);
        _cur_len = 0;
    }

public:
    short _cur_len;
    short _total_len;
    char* _data;
};

class RecvNode : public MsgNode {
public:
    RecvNode(short max_len, short msg_id);

private:
    short _msg_id;

};

class SendNode : public MsgNode {
public:
    SendNode(const char* msg, short max_len, short msg_id);

private:
    short _msg_id;
};



#endif