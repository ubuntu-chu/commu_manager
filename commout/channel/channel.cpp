#include "channel.h"
#include <inner_client.h>
#include <ext_client.h>
#include <inner_server.h>
#include <com.h>
#include <utils.h>

//初始化通信介质
bool channel::init(void)
{
    runinfo_.m_bStatus          = true;
	if ((NULL == protocol_) || (NULL == io_base_)){
	    return false;
	}
	duplex_type_                = io_base_->duplextype_get();
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

bool channel::on_process_aframe(const char * pdata, int len, int iflag)
{
    int i;

    MutexLockGuard lock(mutex_);
    if (0 == iflag){
        for (i = 0; i < len; i++){
            vec_ret_.push_back(pdata[i]);
        }
    }
    status_                             = iflag;
    frame_arrived_                      = true;
    condition_.notify();
    LOG_TRACE << "condition ocurred, wake up";

    return true;
}

int channel::write_sync_inloop(vector<char> &vec, int wait_time, vector<char> **ppvec_ret)
{
    MutexLockGuard lock(mutex_);
    status_                           = 1;
    vec_ret_.clear();
    LOG_TRACE << "condition wait start with " << wait_time << " sec";
    event_loop_->runInLoop(boost::bind(&channel::write_sync, this, vec));
    condition_.waitForSeconds(wait_time);
    if (status_ > 0){
        LOG_TRACE << "condition wait end with time out";
    }else if (status_ == 0){
        LOG_TRACE << "condition wait end with complete";
    }else {
        LOG_TRACE << "condition wait end with error = [" << status_ << "]";
    }
    if (NULL != ppvec_ret){
        *ppvec_ret                      = &vec_ret_;
    }

    return status_;
}
//向通信介质写报文
int channel::write_sync(vector<char> &vec)
{
    if(runinfo_.m_bStatus == false)
        return COMM_NOTINIT;

    utils::log_binary_buf("channel::write", &vec[0], vec.size());
    if (NULL != protocol_){
        return protocol_->write_tochannel(&vec[0], vec.size());
    }
    return on_write(&vec[0], vec.size());
}

int channel::write_inloop(vector<char> &vec)
{
    event_loop_->runInLoop(boost::bind(&channel::write, this, vec));

    return 0;
}
//向通信介质写报文
int channel::write(vector<char> &vec)
{
    if(runinfo_.m_bStatus == false)
        return COMM_NOTINIT;

    utils::log_binary_buf("channel::write", &vec[0], vec.size());
    if (NULL != protocol_){
        return protocol_->write_tochannel(&vec[0], vec.size());
    }
    return on_write(&vec[0], vec.size());
}

//向通信介质写报文
int channel::on_write(const char *pdata, size_t len)
{
    int  rt;
    if((io_base_ == NULL) || (runinfo_.m_bStatus == false))
        return COMM_NOTINIT;

    if(pdata == NULL)
        return COMM_INVALIDPTR;

    utils::log_binary_buf("channel::on_write", pdata, len);
    send_status_set(true);
    rt      =  io_base_->send_package(const_cast<char *>(pdata), len);

    return rt;
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
    if (false == can_receive()){
        LOG_INFO << "drop data because half duplex in send";
        return false;
    }
    utils::log_binary_buf("channel::on_read", pdata, len);
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
        pchannel->io_base_.reset(new ext_client(
                event_loop, serverAddr, pio_tcp_ext_client_node->name_get(), pio_node));
    }
        break;
    case IO_TYPE_INNER_CLIENT:
    {
        io_tcp_client_node *pio_tcp_client_node =
                reinterpret_cast<io_tcp_client_node *>(pio_node);
        const InetAddress serverAddr(pio_tcp_client_node->server_ip_get(),
                pio_tcp_client_node->server_port_get());
        pchannel->io_base_.reset(new inner_client(
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
        pchannel->io_base_.reset(new inner_server(
                event_loop, serverAddr, pio_tcp_server_node->name_get(), pio_node));
    }
        break;
    case IO_TYPE_EXT_COM:
    {
        io_com_ext_node *pio_com_ext_node =
                reinterpret_cast<io_com_ext_node *>(pio_node);
//        pchannel->io_base_ = boost::shared_ptr<io_base>(new com(
//                event_loop, pio_com_ext_node->name_get(), pio_node));
        pchannel->io_base_.reset(new com(
                event_loop, pio_com_ext_node->name_get(), pio_node));
    }
        break;

    default:
        break;
    }
    pchannel->io_base_->channel_set(pchannel);
    pchannel->init();
    pchannel->connect();

	return pchannel;
}

bool channel::contain_protocol(const char *name)
{
    if (NULL == protocol_){
        return false;
    }
    if (0 == strcmp(name, protocol_->name_get().c_str())){
        return true;
    }

    return false;
}


