#include "channel.h"
#include <inner_client.h>
#include <ext_client.h>
#include <inner_server.h>

//初始化通信介质
bool channel::init(void)
{

	LOG_DEBUG;

	return true;
}

//反初始化
bool channel::uninit(void)
{
	LOG_DEBUG;
	return true;
}

//向通信介质写报文
int channel::write(const char *pdata, size_t len)
{
	LOG_DEBUG;
	return 0;
}

//连接通信介质
bool channel::connect(bool brelink)
{
	if (NULL == io_base_){
	    return false;
	}

	return io_base_->connect(brelink);
}

//断开通信介质，
bool channel::disconnect(void)
{
	if (NULL == io_base_){
	    return false;
	}

	return io_base_->disconnect();
}

channel *channel::create_channel(const io_node *pio_node_const)
{
    channel *pchannel;
    int     io_node_type;
    io_node *pio_node   = const_cast<io_node *>(pio_node_const);
    EventLoop* event_loop;

	LOG_INFO << "create channel with io_type" << pio_node->type_get();
	pchannel                            = new channel;
    pchannel->event_loopthread_ = boost::shared_ptr<EventLoopThread>(new EventLoopThread(NULL));
    event_loop                          = pchannel->event_loopthread_->startLoop();
    io_node_type                        = io_node::io_type_get(pio_node->type_get());

    //还要有规约的创建步骤
    switch (io_node_type){
    case IO_TYPE_EXT_CLIENT:
    {
        io_tcp_ext_client_node *pio_tcp_ext_client_node =
                reinterpret_cast<io_tcp_ext_client_node *>(pio_node);
        const InetAddress serverAddr(pio_tcp_ext_client_node->server_ip_get(),
                pio_tcp_ext_client_node->server_port_get());
        pchannel->io_base_ = boost::shared_ptr<io_base>(new ext_client(
                event_loop, serverAddr, pio_tcp_ext_client_node->name_get(), pio_node));
    }
        break;
    case IO_TYPE_INNER_CLIENT:
    {
        io_tcp_client_node *pio_tcp_client_node =
                reinterpret_cast<io_tcp_client_node *>(pio_node);
        const InetAddress serverAddr(pio_tcp_client_node->server_ip_get(),
                pio_tcp_client_node->server_port_get());
        pchannel->io_base_ = boost::shared_ptr<io_base>(new inner_client(
                event_loop, serverAddr, pio_tcp_client_node->name_get(), pio_node));
    }
        break;
    case IO_TYPE_INNER_SERVER:
    {
        io_tcp_server_node *pio_tcp_server_node =
                reinterpret_cast<io_tcp_server_node *>(pio_node);
        const InetAddress serverAddr(pio_tcp_server_node->server_ip_get(),
                pio_tcp_server_node->server_port_get());
        pchannel->io_base_ = boost::shared_ptr<io_base>(new inner_server(
                event_loop, serverAddr, pio_tcp_server_node->name_get(), pio_node));
    }
        break;
    case IO_TYPE_EXT_COM:
        break;

    default:
        break;
    }

	return pchannel;
}

bool channel::on_read(const char *pdata, size_t len, int flag)
{

    return true;
}


