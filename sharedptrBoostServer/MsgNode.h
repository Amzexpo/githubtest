#ifndef _MsgNode_H_
#define _MsgNode_H_

#include <iostream>
#include <boost/asio.hpp>

#include "Server.h"
#include "Session.h"

class MsgNode{
    friend class Session;
public:
    MsgNode(char *msg, int max_len) {
        _max_len = max_len;
        _data = new char[max_len];
        memcpy(_data, msg, max_len);
    }
    ~MsgNode() {
        delete[] _data;
    }
private:
    int _cur_len;
    int _max_len;
    char* _data;
};







#endif