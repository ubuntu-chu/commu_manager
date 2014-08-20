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

//目前的实现方案中 接收的帧最多只有一条 没有缓存机制 若要有缓存机制 则需要重新定义函数的返回值及帧的存储方式
//iflag = 0  接收到完整的一帧
//iflag < 0   接收到错误帧
bool channel::on_process_aframe(const char * pdata, int len, int iflag)
{
    int i;

    MutexLockGuard lock(mutex_);
    //接收到完整一帧
    if (0 == iflag){
        //新的一帧已经到来   将之前尚未处理的帧清除
        vec_ret_.clear();
        for (i = 0; i < len; i++){
            vec_ret_.push_back(pdata[i]);
        }
    //接收到错误帧
    }else {
        LOG_WARN << io_node_name_get()
                << "channel::on_process_aframe: a error frame receive; iflag = [" << iflag <<"]";
    }
    frame_arrived_                      = true;
    //将帧的状态暂存
    frame_flag_                         = iflag;
    status_                             = frame_flag_;
    //判断之前是否调用过 write_sync_inloop函数
    if (1 == write_sync_inloop_called_){
        condition_.notify();
    }
    LOG_TRACE << io_node_name_get() << ": condition ocurred, wake up status_ = [" << status_ << "]";

    return true;
}

//用于查询通道是否接收到一帧数据   如果接收到 则把帧数据存放到vec中
//返回值： enum_CH_FETCH_AFRAME            接收到完整一帧
//        enum_CH_FETCH_ERR_FRAME            接收到错误帧
//        enum_CH_FETCH_NO_FRAME            没有接收到帧
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

//向通信介质写报文  接口函数
//同步写操作
//先将数据发送  然后最多等到 wait_time 秒， 判断在wait_time时间内，是否接到返回数据
//status_ > 0   调用超时 write_sync_inloop在wait_time时间内 没有接收到返回数据
//status_ = 0   调用成功 write_sync_inloop在wait_time时间内 接收到完整的一帧返回数据
//status_ < 0   调用失败 write_sync_inloop在wait_time时间内 接收到错误的一帧返回数据
int channel::write_sync_inloop(vector<char> &vec, int wait_time, vector<char> **ppvec_ret)
{
    int     rt;

    MutexLockGuard lock(mutex_);
    //status_默认为超时状态
    status_                             = 1;
    write_sync_inloop_called_           = 1;
    if (enum_WORK_TYPE_HALF_DUPLEX == duplex_type_){
        //半双工通讯  清空之前接收到的返回值
        frame_arrived_                  = false;
        vec_ret_.clear();
    }
    LOG_TRACE << io_node_name_get() << ": condition wait start with " << wait_time << " sec";
    //在event_loop中执行channel::write函数
    event_loop_->runInLoop(boost::bind(&channel::write, this, vec));
    //判断在调用之前 是否已经接收到一帧数据
    if (false == frame_arrived_){
        condition_.waitForSeconds(wait_time);
    }else {
        //frame_arrived_为true时 代表在调用写之前已经有一帧接收到 帧的状态存放在frame_flag_中
        status_                         = frame_flag_;
    }
    //判断执行状态
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

//向通信介质写报文  接口函数
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

    //判断通道的io_base状态
    if (false == io_base_->connected()){
        LOG_WARN << "channel::on_write quit due to <" << io_node_name_get()
                << "> no connect";
        return COMM_DISCONNECT;
    }

    utils::log_binary_buf_trace("channel::write", &vec[0], vec.size());
    //若通道上关联协议的话  则调用协议的write_tochannel函数 来发送报文
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
    //io_base_->send_package完成报文发送
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

//本次应用 未使用
bool channel::handle_timer(void)
{
	if (NULL != protocol_){
	    return protocol_->handle_timer();
	}

    return true;
}

//通道报文接收函数
bool channel::on_read(const char *pdata, int len, int flag)
{
    if (false == can_receive()){
        LOG_WARN << "drop data because half duplex in send";
        return false;
    }
    //若通道关联协议的话  则调用协议的protocol_->read_frchannel函数进行处理
	if (NULL != protocol_){
	    return protocol_->read_frchannel(pdata, len, flag);
	}else {
	    utils::log_binary_buf("channel::on_read; protocol_ = NULL", pdata, len);
	}

    return true;
}

//创建通道
channel *channel::channel_create(const io_node *pio_node_const)
{
    channel *pchannel;
    int     io_node_type;
    io_node *pio_node   = const_cast<io_node *>(pio_node_const);
    EventLoop* event_loop;

	pchannel                            = new channel();
	//EventLoopThread为一个运行event_loop事件循环的线程
    pchannel->event_loopthread_.reset(new EventLoopThread(NULL));
    //启动线程运行
    event_loop                          = pchannel->event_loopthread_->startLoop();
    pchannel->event_loop_               = event_loop;
    io_node_type                        = io_node::io_type_get(pio_node->type_get());

    //创建协议
    pchannel->protocol_.reset(protocol::protocol_create(pio_node->protocol_get()));
//    pchannel->protocol_         = boost::shared_ptr<protocol>(
//                                        protocol::protocol_create(pio_node->protocol_get()));
    if (NULL == pchannel->protocol_){
        LOG_ERROR << "pchannel->protocol == NULL";
    }
//    CHECK_NOTNULL(pchannel->protocol_);
    //设置协议所关联的通道
    pchannel->protocol_->channel_set(pchannel);
    //开启1s定时器  未使用
//    pchannel->event_loop_->runEvery(1, boost::bind(&channel::handle_timer, pchannel));

    //创建io_base类型
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
    //未使用
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
    //未使用
    case IO_TYPE_INNER_SERVER:
    {
        io_tcp_server_node *pio_tcp_server_node =
                reinterpret_cast<io_tcp_server_node *>(pio_node);
        const InetAddress serverAddr(pio_tcp_server_node->server_port_get(), false);
        pchannel->io_base_.reset(new inner_server(
                event_loop, serverAddr, pio_tcp_server_node->name_get(), pio_node));
    }
        break;
    case IO_TYPE_EXT_COM:
    {
        io_com_ext_node *pio_com_ext_node =
                reinterpret_cast<io_com_ext_node *>(pio_node);
        pchannel->io_base_.reset(new com(
                event_loop, pio_com_ext_node->name_get(), pio_node));
    }
        break;

    default:
        break;
    }
    //设置io_base所关联的通道
    pchannel->io_base_->channel_set(pchannel);
    pchannel->init();
    pchannel->connect();

	return pchannel;
}

//判断通道是否关联了名字为nade的协议
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

//获取通道下io_base下所挂载的设备个数
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
    //遍历链表 得到链表上节点个数
    list_for_each(pos, plist_head){
        no++;
    }

    return no;
}

//获取通道下io_base下所挂载的设备链表头
list_head_t *channel::device_list_head_get(void)
{
    if (NULL == io_base_){
        return NULL;
    }
    list_head_t * plist_head    = io_base_->io_node_get()->device_list_head_get();

    return plist_head;
}

//获取通道下io_base所关联的io_base下所挂载的设备链表头
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

