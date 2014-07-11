#include "ext_client.h"


ext_client::ext_client(EventLoop* loop, const InetAddress& listenAddr, const char *name) :
        loop_(loop), client_(loop, listenAddr, name)
{
    client_.setConnectionCallback(
            boost::bind(&ext_client::onConnection, this, _1));
    client_.setMessageCallback(
            boost::bind(&ext_client::onMessage, this, _1, _2, _3));
    // client_.enableRetry();
}


void ext_client::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");

    if (!conn->connected())
        loop_->quit();
}

void ext_client::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
        Timestamp receiveTime)
{
    buf->retrieveAll();
}


