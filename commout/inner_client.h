#ifndef _INNER_CLIENT_H
#define _INNER_CLIENT_H

#include <includes/includes.h>

using namespace muduo;
using namespace muduo::net;

class inner_client: boost::noncopyable {
public:
    inner_client(EventLoop* loop, const InetAddress& listenAddr, const char *name);
    void connect()
    {
        client_.connect();
    }

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);

    EventLoop* loop_;
    TcpClient client_;
};


#endif

