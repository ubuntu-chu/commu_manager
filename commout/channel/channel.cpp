#include "channel.h"
#include <inner_client.h>
#include <ext_client.h>
#include <inner_server.h>
#include <com.h>
#include <io_node.h>
#include <utils.h>

//初始化通信介质
bool channel::init(void)
{
    runinfo_.m_bStatus                  = true;
	if ((NULL == protocol_) || (NULL == io_base_)){
	    return false;
	}
	duplex_type_                        = io_base_->duplextype_get();
	status_                             = 0;
    frame_arrived_                      = false;
    write_sync_inloop_called_           = 0;
    protocol_->init();
	io_base_->init();
	//默认开启通道电源
	power_on();

	return true;
}

//反初始化
bool channel::uninit(void)
{
	if ((NULL == protocol_) || (NULL == io_base_)){
	    return false;
	}

    protocol_->uninit();
    power_off();
	return io_base_->uninit();
}

//目前的实现方案中 接收的帧最多只有一条 没有缓存机制 若要有缓存机制 则需要重新定义函数的返回值及镇的存储方式
//iflag == 0  接收到完整的一帧
//iflag < 0   接收到错误帧
bool channel::on_process_aframe(const char * pdata, int len, int iflag)
{
    int i;

    MutexLockGuard lock(mutex_);
    if (0 == iflag){
        //新的一帧已经到来   将之前尚未处理的帧清除
        vec_ret_.clear();
        for (i = 0; i < len; i++){
            vec_ret_.push_back(pdata[i]);
        }
    }else {
        LOG_WARN << io_node_name_get()
                << "channel::on_process_aframe: a error frame receive; iflag = [" << iflag <<"]";
    }
    frame_arrived_                      = true;
    frame_flag_                         = iflag;
    status_                             = frame_flag_;
    //判断之前是否调用过 write_sync_inloop函数
    if (1 == write_sync_inloop_called_){
        condition_.notify();
    }
    LOG_TRACE << io_node_name_get() << ": condition ocurred, wake up status_ = [" << status_ << "]";

    return true;
}

enum channel_fetch channel::fetch(vector<char> &vec)
{
    if (frame_arrived_ == true){
        MutexLockGuard lock(mutex_);

        //接收到完整帧
        if (0 == frame_flag_){
            vec                         = vec_ret_;
        }
        vec_ret_.clear();
        frame_arrived_                  = false;

        return (0 == frame_flag_)?(enum_CH_FETCH_AFRAME):(enum_CH_FETCH_ERR_FRAME);
    }

    return enum_CH_FETCH_NO_FRAME;
}

int channel::write_sync_inloop(vector<char> &vec, int wait_time, vector<char> **ppvec_ret)
{
    int     rt;

    MutexLockGuard lock(mutex_);
    //status_默认为超时状态
    status_                             = 1;
    write_sync_inloop_called_           = 1;
    if (enum_WORK_TYPE_HALF_DUPLEX == duplex_type_){
        frame_arrived_                  = false;
        vec_ret_.clear();
    }
    LOG_TRACE << io_node_name_get() << ": condition wait start with " << wait_time << " sec";
//    LOG_WARN << io_node_name_get() << ": condition wait start with " << wait_time << " sec";
    event_loop_->runInLoop(boost::bind(&channel::write, this, vec));
    if (false == frame_arrived_){
        condition_.waitForSeconds(wait_time);
    }else {
        //frame_arrived_为true时 代表在调用写之前已经有一帧接收到 帧的状态存放在frame_flag_中
        status_                         = frame_flag_;
    }
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

    frame_arrived_                      = false;
    rt                                  = status_;
    //恢复默认值
    write_sync_inloop_called_           = 0;

    return rt;
}

int channel::write_inloop(vector<char> &vec)
{
    //此处bind对vec是拷贝行为
    event_loop_->runInLoop(boost::bind(&channel::write, this, vec));

    return 0;
}
//向通信介质写报文
int channel::write(vector<char> &vec)
{
    if(runinfo_.m_bStatus == false)
        return COMM_NOTINIT;

    if (false == io_base_->connected()){
        LOG_WARN << "channel::on_write quit due to <" << io_node_name_get()
                << "> no connect";
        return COMM_DISCONNECT;
    }

    utils::log_binary_buf_trace("channel::write", &vec[0], vec.size());
    if (NULL != protocol_){
        return protocol_->write_tochannel(&vec[0], vec.size());
    }
//    return on_write(&vec[0], vec.size());
    return on_write(&*vec.begin(), vec.size());
}

//向通信介质写报文
int channel::on_write(const char *pdata, size_t len)
{
    int  rt;
    if((io_base_ == NULL) || (runinfo_.m_bStatus == false))
        return COMM_NOTINIT;

    if(pdata == NULL)
        return COMM_INVALIDPTR;

    utils::log_binary_buf_trace("channel::on_write", pdata, len);
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
        LOG_WARN << "drop data because half duplex in send";
        return false;
    }
	if (NULL != protocol_){
	    return protocol_->read_frchannel(pdata, len, flag);
	}else {
	    utils::log_binary_buf("channel::on_read; protocol_ = NULL", pdata, len);
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
    pchannel->event_loopthread_.reset(new EventLoopThread(NULL));
//    pchannel->event_loopthread_ = boost::shared_ptr<EventLoopThread>(new EventLoopThread(NULL));
    event_loop                          = pchannel->event_loopthread_->startLoop();
    pchannel->event_loop_               = event_loop;
    io_node_type                        = io_node::io_type_get(pio_node->type_get());

    pchannel->protocol_.reset(protocol::protocol_create(pio_node->protocol_get()));
//    pchannel->protocol_         = boost::shared_ptr<protocol>(
//                                        protocol::protocol_create(pio_node->protocol_get()));
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

int channel::device_no_get(void)
{
    int no                          = 0;

    list_node_t *pos;
    //获取此通道下io下所挂接设备
    list_head_t * plist_head;

    if (NULL == io_base_){
        return no;
    }
    plist_head                      = io_base_->io_node_get()->device_list_head_get();
    list_for_each(pos, plist_head){
        no++;
    }

    return no;
}

list_head_t *channel::device_list_head_get(void)
{
    if (NULL == io_base_){
        return NULL;
    }
    list_head_t * plist_head    = io_base_->io_node_get()->device_list_head_get();

    return plist_head;
}


list_head_t *channel::device_maped_list_head_get(void)
{
    io_base *pio_base           = io_base_get();
    io_node *pio_node           = pio_base->io_node_get();
    io_node *pio_node_map       = pio_node->io_node_map_get();

    //获取此通道下io下所挂接设备
    //同一个io下所有设备的class type必须相同
    list_head_t * plist_head  = pio_node_map->device_list_head_get();

    return plist_head;
}

