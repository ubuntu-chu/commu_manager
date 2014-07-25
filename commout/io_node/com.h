#ifndef _COM_H
#define _COM_H

#include <includes/includes.h>
#include "io_base.h"

using namespace muduo;
using namespace muduo::net;

class com: public io_base, boost::noncopyable {
public:
    com(EventLoop* loop, const char *name, io_node *pio_node = NULL);

    //初始化通信介质
    virtual bool init(void);
    bool _init(void);

    //反初始化
    virtual bool uninit(void);
    bool _uninit(void);

    //向通道写报文
    virtual int send_data(char *pdata, size_t len);
    int _send_data(char *pdata, size_t len);

    void handle_read(Timestamp receiveTime);

private:
    EventLoop* loop_;
    boost::scoped_ptr<Channel> channel_;
    struct termios  termios_;
    int         fd_;
    Buffer inputBuffer_;



};


#endif
