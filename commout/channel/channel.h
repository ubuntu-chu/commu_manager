#ifndef _CHANNEL_H
#define _CHANNEL_H

#include <io_node.h>
#include <includes/includes.h>
#include <io_base.h>

using namespace muduo;
using namespace muduo::net;

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
    virtual ~channel(){}

    //初始化通信介质
    virtual bool init(void);

    //反初始化
    virtual bool uninit();

    //向通信介质写报文
    int write(const char *pdata, size_t len);

    //连接通信介质
    virtual bool connect(bool brelink = true);

    //断开通信介质，
    virtual bool disconnect(void);

    channel_runinfo &get_runinfo()
    {
        return runinfo_;
    }

    static channel *create_channel(const io_node *pio_node_const);

protected:
    virtual bool on_read(const char *pdata, size_t len, int flag);

private:
    channel_runinfo         runinfo_;
    boost::shared_ptr<io_base> io_base_;
    boost::shared_ptr<EventLoopThread> event_loopthread_;
};

#endif
