#include "inner_server.h"


inner_server::inner_server(muduo::net::EventLoop* loop,
        const muduo::net::InetAddress& listenAddr, const char *name) :
        server_(loop, listenAddr, name)
{
    server_.setConnectionCallback(
            boost::bind(&inner_server::onConnection, this, _1));
    server_.setMessageCallback(
            boost::bind(&inner_server::onMessage, this, _1, _2, _3));
}

void inner_server::start()
{
    server_.start();
}

void inner_server::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "inner_server - " << conn->peerAddress().toIpPort() << " -> "
            << conn->localAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
}

void inner_server::onMessage(const TcpConnectionPtr& conn,
        Buffer* buf, Timestamp time)
{
    string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
            << "data received at " << time.toString();
    conn->send(msg);
}




