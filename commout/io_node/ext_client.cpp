#include "ext_client.h"


ext_client::ext_client(EventLoop* loop,
        const InetAddress& listenAddr, const char *name, const io_node *pio_node) :
        io_base(pio_node), loop_(loop), client_(loop, listenAddr, name)
{
    io_tcp_ext_client_node *pio_tcp_ext_client_node =
            reinterpret_cast<io_tcp_ext_client_node *>(const_cast<io_node *>(pio_node));
    LOG_INFO << "IO_TYPE_EXT_CLIENT create; serverip = " << listenAddr.toIpPort()\
            << " local ip = " << pio_tcp_ext_client_node->client_ip_get();


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

    MutexLockGuard lock(mutex_);
    if (conn->connected()) {
        connection_ = conn;
        connection_->send("nihao");
    } else {
        connection_.reset();
    }
}

void ext_client::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
        Timestamp receiveTime)
{
//    buf->retrieveAll();
//    string msg(buf->retrieveAllAsString());
    char print_buf[1000];
    unsigned int i;
    int len                  = 0;;
    const char* begin       = buf->peek();

    for (i = 0; i < buf->readableBytes(); i++){
        len += snprintf(&print_buf[len], sizeof(print_buf)-len,
                " %02x", static_cast<uint8>(begin[i]));
    }


    LOG_INFO << conn->name() << " msg :(" << print_buf << ") " << i << " bytes, "
            << "received at " << receiveTime.toString();
#if 0
    LOG_INFO << conn->name() << "msg :" << msg << msg.size() << " bytes, "
            << "received at " << receiveTime.toString();
#endif
//    conn->send(msg);

    io_base::on_read(begin, i, 0);
    buf->retrieveAll();
}


