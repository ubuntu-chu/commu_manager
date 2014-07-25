#ifndef _EXT_CLIENT_H
#define _EXT_CLIENT_H

#include <includes/includes.h>
#include "io_base.h"

using namespace muduo;
using namespace muduo::net;

class ext_client: public io_base, boost::noncopyable {
public:
    ext_client(EventLoop* loop, const InetAddress& listenAddr,
            const char *name, io_node *pio_node = NULL);

    //连接通信介质
    virtual bool connect(bool brelink = true)
    {
        LOG_TRACE;
        client_.connect();
        return true;
    }

    //断开通信介质，
    virtual bool disconnect(void)
    {
        LOG_TRACE;
        client_.disconnect();
        return true;
    }

    //向通道写报文
    virtual int send_data(char *pdata, size_t len)
    {
        int actual_len              = 0;
        MutexLockGuard lock(mutex_);
        if (connection_){
          connection_->send(pdata, len);
          actual_len                = len;
        }
        send_status_end(actual_len);

        return len;
    }

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);

    EventLoop* loop_;
    TcpClient client_;
    TcpConnectionPtr connection_;
    MutexLock mutex_;

};


#endif
