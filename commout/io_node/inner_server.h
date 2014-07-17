#ifndef _INNER_SERVER_H
#define _INNER_SERVER_H

#include <includes/includes.h>
#include "io_base.h"

using namespace muduo;
using namespace muduo::net;
using std::string;

class inner_server: public io_base, boost::noncopyable {
public:
    inner_server(EventLoop* loop, const InetAddress& listenAddr,
            const char *name, const io_node *pio_node = NULL);
    void start();  // calls server_.start();

    //初始化通信介质
    virtual bool init(void)
    {
        server_.start();
        return true;
    }

    //反初始化
    virtual bool uninit()
    {
        return true;
    }


private:
    void onConnection(const TcpConnectionPtr& conn);

    void onMessage(const TcpConnectionPtr& conn,
            Buffer* buf, Timestamp time);

    TcpServer server_;
};



#endif

