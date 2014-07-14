#ifndef _EXT_CLIENT_H
#define _EXT_CLIENT_H

#include <includes/includes.h>
#include "io_base.h"

using namespace muduo;
using namespace muduo::net;

class ext_client: public io_base, boost::noncopyable {
public:
    ext_client(EventLoop* loop, const InetAddress& listenAddr,
            const char *name, const io_node *pio_node = NULL);
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
