#include "inner_client.h"


inner_client::inner_client(EventLoop* loop, const InetAddress& listenAddr, const char *name) :
        loop_(loop), client_(loop, listenAddr, name)
{
    client_.setConnectionCallback(
            boost::bind(&inner_client::onConnection, this, _1));
    client_.setMessageCallback(
            boost::bind(&inner_client::onMessage, this, _1, _2, _3));
    // client_.enableRetry();
}


void inner_client::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");

    if (!conn->connected())
        loop_->quit();
}

void inner_client::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
        Timestamp receiveTime)
{
    buf->retrieveAll();
}


