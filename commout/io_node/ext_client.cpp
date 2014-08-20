#include "ext_client.h"
#include <utils.h>


ext_client::ext_client(EventLoop* loop,
        const InetAddress& listenAddr, const char *name, io_node *pio_node) :
        io_base(pio_node), loop_(loop), client_(loop, listenAddr, name)
{
    io_tcp_ext_client_node *pio_tcp_ext_client_node =
            reinterpret_cast<io_tcp_ext_client_node *>(const_cast<io_node *>(pio_node));
    LOG_INFO << "IO_TYPE_EXT_CLIENT create; serverip = " << listenAddr.toIpPort()\
            << " local ip = " << pio_tcp_ext_client_node->client_ip_get();

    //设置链接建立回调函数
    client_.setConnectionCallback(
            boost::bind(&ext_client::onConnection, this, _1));
    //设置消息到达时回调函数
    client_.setMessageCallback(
            boost::bind(&ext_client::onMessage, this, _1, _2, _3));
    //是能客户端重试功能  网络断线时，客户端会进行重试
    client_.enableRetry();
}


void ext_client::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");

    MutexLockGuard lock(mutex_);
    if (conn->connected()) {
        //记录此客户端连接
        connection_             = conn;
        connected_              = true;
    } else {
        //释放此客户端连接
        connection_.reset();
        connected_              = false;
    }
}

//消息达到回调函数
void ext_client::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
        Timestamp receiveTime)
{
    LOG_DEBUG << conn->name() << "received at " << receiveTime.toString();

    //推入到channel
    io_base::on_read(buf->peek(), buf->readableBytes(), 0);
    //清空buf中的数据内容
    buf->retrieveAll();
}


