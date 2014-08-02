#ifndef _CHANNEL_H
#define _CHANNEL_H

#include <io_node.h>
#include <device_node.h>
#include <includes/includes.h>
#include <io_base.h>
#include <protocol.h>

using namespace muduo;
using namespace muduo::net;
using muduo::net::Buffer;

enum{
    COMM_ERROR          =   -1,
    COMM_OK             =   0,

    //正常信息
    COMM_INIT           =   100,        //通道初始化（非连接通道，或服务端）
    COMM_REINIT         =   101,        //通道重新初始化（非连接通道，或服务端）

    COMM_CLOSE          =   103,        //通道关闭

    COMM_CONNECT        =   105,        //通道连接成功（面向连接）
    COMM_ACCEPT         =   107,        //接收新的客户端（面向连接）
    COMM_WAITCONNECT    =   108,        //等待客户端连接
    COMM_DISACCEPT      =   109,        //不接收新的客户端（面向连接）

    //错误信息
    COMM_NOACTIVECH     =   -1000,       //没有处于工作状态的通道
    COMM_NOTINIT        =   -1001,       //通道未初始化
    CHANNEL_NOTINIT     =   -1002,       //通道未初始化

    COMM_INVALIDPTR     =   -2000,       //无效的指针
    COMM_DISCONNECT     =   -2001,        //通道断开（面向连接）
};

class channel_runinfo{
public:
    bool            m_bStatus;             //通信是否正常
    bool            m_bConnected;           //是否连接（面向连接的通道有效）
    bool            m_bsend;                //发送状态
    int             m_nRcvByteTotal;        //接受数据包累计
    int             m_nSndByteTotal;        //发送数据包累计
    int             m_nRcvByteAverage;      //每秒接受数据包
    int             m_nSndByteAverage;      //每秒发送数据包
};


class channel_worktype {
public:
    bool            m_bAutoSwitch;  //面向连接的通道断开是否自动切换
    bool            m_AutoToMain;   //备用通道工作时是否自动切换到主通道
    int             m_nScanSpan;    //自动切换时间间隔(ms)
    int             m_iWorkType;    //通道组工作方式
    int             m_iRetryTimes;  //断开时重连次数
};

enum channel_fetch{
    enum_CH_FETCH_ERR_FRAME = -1,
    enum_CH_FETCH_AFRAME    = 0,
    enum_CH_FETCH_NO_FRAME = 1,

};

class channel:boost::noncopyable{
public:
    channel():mutex_(),
        condition_(mutex_),
        status_(0),
        frame_arrived_(0)
    {
    }
    ~channel(){}

    //初始化通信介质
    bool init(void);

    //反初始化
    bool uninit();

    //获取函数
    //返回值： false 表示通道暂未接收到数据帧
    //       true   表示通道已接受到数据帧    数据帧通过容器返回
    enum channel_fetch fetch(vector<char> &vec);

    //向通信介质写报文   同步版本
    //返回值： > 0  执行超时
    //       = 0  执行成功
    //       < 0  执行错误
    int write_sync_inloop(vector<char> &vec, int wait_time, vector<char> **ppvec_ret);

    //向通信介质写报文
    int write_inloop(vector<char> &vec);

    int on_write(const char *pdata, size_t len);
    bool on_read(const char *pdata, int len, int flag);

    bool on_process_aframe(const char * pdata, int len, int iflag = 0);

    //连接通信介质
    bool connect(bool brelink = true);

    //断开通信介质，
    bool disconnect(void);

    channel_runinfo &get_runinfo()
    {
        return runinfo_;
    }

    bool handle_timer(void);

    static channel *channel_create(const io_node *pio_node_const);

    bool contain_protocol(const char *name);

    int duplextype_get(void)
    {
        return duplex_type_;
    }

    void send_status_set(bool status)
    {
        runinfo_.m_bsend                    = status;
    }
    bool send_status_get(void)
    {
        return runinfo_.m_bsend;
    }
    bool can_receive(void)
    {
        if ((enum_WORK_TYPE_HALF_DUPLEX == duplex_type_)
                && (true == runinfo_.m_bsend)){
            return false;
        }

        return true;
    }

    bool power_on(void)
    {
        return power_ctrl('1');
    }
    bool power_off(void)
    {
        return power_ctrl('0');
    }
    int power_get(void)
    {
        return power_statue_ - '0';
    }

    protocol *protocol_get(void)
    {
        return protocol_.get();
    }
    io_base *io_base_get(void)
    {
        return io_base_.get();
    }

    //获取此通道的io_base下所挂在设备的数目
    int device_no_get(void);
    list_head_t *device_list_head_get(void);
    list_head_t *device_maped_list_head_get(void);

private:
    int write(vector<char> &vec);
	bool power_ctrl(char value)
	{
	    power_statue_                   = value;
	    if (NULL == io_base_){
	        return false;
	    }
        return io_base_->power_ctrl(value);
	}
	const char *io_node_name_get(void)
	{
	    return io_base_->io_node_get()->name_get();
	}


    channel_runinfo         runinfo_;
    int                     duplex_type_;
    int                     power_statue_;

//    channel_worktype        work_type;
    boost::shared_ptr<io_base> io_base_;
    boost::shared_ptr<protocol> protocol_;
    boost::shared_ptr<EventLoopThread> event_loopthread_;
    EventLoop               *event_loop_;

    mutable MutexLock      mutex_;
    Condition               condition_;
    int                     status_;
    sig_atomic_t            frame_arrived_;
    sig_atomic_t            frame_flag_;
    sig_atomic_t            write_sync_inloop_called_;
    vector<char>            vec_ret_;
};

#endif
