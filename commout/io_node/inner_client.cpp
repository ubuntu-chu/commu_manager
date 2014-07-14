#include "inner_client.h"


inner_client::inner_client(EventLoop* loop,
        const InetAddress& listenAddr, const char *name, const io_node *pio_node) :
        io_base(pio_node), loop_(loop), client_(loop, listenAddr, name)
{
    io_tcp_client_node *pio_tcp_client_node =
            reinterpret_cast<io_tcp_client_node *>(const_cast<io_node *>(pio_node));
    LOG_INFO << "IO_TYPE_INNER_CLIENT create; serverip = " << listenAddr.toIpPort()\
            << " local ip = " << pio_tcp_client_node->client_ip_get();
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

    MutexLockGuard lock(mutex_);
    if (conn->connected()) {
        connection_ = conn;
    } else {
        connection_.reset();
    }
}

void inner_client::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
        Timestamp receiveTime)
{
    buf->retrieveAll();
}


