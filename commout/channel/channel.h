#ifndef _CHANNEL_H
#define _CHANNEL_H

#include <io_node.h>
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
    COMM_DISCONNECT     =   106,        //通道断开（面向连接）
    COMM_ACCEPT         =   107,        //接收新的客户端（面向连接）
    COMM_WAITCONNECT    =   108,        //等待客户端连接
    COMM_DISACCEPT      =   109,        //不接收新的客户端（面向连接）

    //错误信息
    COMM_NOACTIVECH     =   -1000,       //没有处于工作状态的通道
    COMM_NOTINIT        =   -1001,       //通道未初始化
    CHANNEL_NOTINIT     =   -1002,       //通道未初始化

    COMM_INVALIDPTR     =   -2000,       //无效的指针
};

class channel_runinfo{
public:
    bool            m_bStatus;             //通信是否正常
    bool            m_bConnected;           //是否连接（面向连接的通道有效）
    int             m_nRcvByteTotal;        //接受数据包累计
    int             m_nSndByteTotal;        //发送数据包累计
    int             m_nRcvByteAverage;      //每秒接受数据包
    int             m_nSndByteAverage;      //每秒发送数据包
};

class CChannWorkType {
public:
    bool            m_bAutoSwitch;  //面向连接的通道断开是否自动切换
    bool            m_AutoToMain;   //备用通道工作时是否自动切换到主通道
    int             m_nScanSpan;    //自动切换时间间隔(ms)
    int             m_iWorkType;    //通道组工作方式
    int             m_iRetryTimes;  //断开时重连次数
};

class channel:boost::noncopyable{
public:
    channel(){}
    ~channel(){}

    //初始化通信介质
    bool init(void);

    //反初始化
    bool uninit();

    //向通信介质写报文
    int write(const char *pdata, size_t len);
    bool on_read(const char *pdata, int len, int flag);

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

private:
    channel_runinfo         runinfo_;
    boost::shared_ptr<io_base> io_base_;
    boost::shared_ptr<protocol> protocol_;
    boost::shared_ptr<EventLoopThread> event_loopthread_;
    EventLoop               *event_loop_;
};

#endif
