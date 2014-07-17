#include "channel.h"
#include <inner_client.h>
#include <ext_client.h>
#include <inner_server.h>

//初始化通信介质
bool channel::init(void)
{
    runinfo_.m_bStatus          = true;
	if ((NULL == protocol_) || (NULL == io_base_)){
	    return false;
	}
    protocol_->init();
	return io_base_->init();
}

//反初始化
bool channel::uninit(void)
{
	if ((NULL == protocol_) || (NULL == io_base_)){
	    return false;
	}

    protocol_->uninit();
	return io_base_->uninit();
}

//向通信介质写报文
int channel::write(const char *pdata, size_t len)
{
    if(io_base_ == NULL || runinfo_.m_bStatus == false)
        return COMM_NOTINIT;

    if(pdata == NULL)
        return COMM_INVALIDPTR;

    return io_base_->send_package(const_cast<char *>(pdata), len);
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

bool channel::handle_timer(void)
{
	if (NULL != protocol_){
	    return protocol_->handle_timer();
	}

    return true;
}

bool channel::on_read(const char *pdata, int len, int flag)
{
	if (NULL != protocol_){
	    return protocol_->read_frchannel(pdata, len, flag);
	}

    return true;
}

channel *channel::channel_create(const io_node *pio_node_const)
{
    channel *pchannel;
    int     io_node_type;
    io_node *pio_node   = const_cast<io_node *>(pio_node_const);
    EventLoop* event_loop;

	pchannel                            = new channel();
    pchannel->event_loopthread_ = boost::shared_ptr<EventLoopThread>(new EventLoopThread(NULL));
    event_loop                          = pchannel->event_loopthread_->startLoop();
    pchannel->event_loop_               = event_loop;
    io_node_type                        = io_node::io_type_get(pio_node->type_get());

    pchannel->protocol_         = boost::shared_ptr<protocol>(
                                        protocol::protocol_create(pio_node->protocol_get()));
    if (NULL == pchannel->protocol_){
        LOG_ERROR << "pchannel->protocol == NULL";
    }
//    CHECK_NOTNULL(pchannel->protocol_);
    pchannel->protocol_->channel_set(pchannel);
    //开启1s定时器
//    pchannel->event_loop_->runEvery(1, boost::bind(&channel::handle_timer, pchannel));

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
//        const InetAddress serverAddr(pio_tcp_server_node->server_ip_get(),
//                pio_tcp_server_node->server_port_get());
        const InetAddress serverAddr(pio_tcp_server_node->server_port_get(), false);
        pchannel->io_base_ = boost::shared_ptr<io_base>(new inner_server(
                event_loop, serverAddr, pio_tcp_server_node->name_get(), pio_node));
    }
        break;
    case IO_TYPE_EXT_COM:
        break;

    default:
        break;
    }
    pchannel->io_base_->channel_set(pchannel);
    pchannel->init();
    pchannel->connect();

	return pchannel;
}



