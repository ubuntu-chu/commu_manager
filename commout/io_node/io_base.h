#ifndef _IO_BASE_H
#define _IO_BASE_H

#include <includes/includes.h>
#include <io_node.h>
using muduo::net::Buffer;

class channel;
//io基类  io基类一定会关联一个channel 和 一个io_node_
//io完成具体的发送、接收操作   把接收到的数据传递给channel   把channel传递过来的数据发送出去
class io_base{
public:
    io_base(io_node *pio_node):connected_(false), pio_node_(pio_node){}
    virtual ~io_base(){}

    //初始化通信介质
    virtual bool init(void);

    //反初始化
    virtual bool uninit();

    //关连通道
    void channel_set(channel *pchannel)
    {
        pchannel_                   = pchannel;
    }

    //连接通信介质
    virtual bool connect(bool brelink = true);
    //断开通信介质，
    virtual bool disconnect(void);
    bool connected(void)
    {
        return connected_;
    }

    //报文拆帧后发送
    int send_package(char *pdata, size_t len);

    //向通道写报文
    virtual int send_data(char *pdata, size_t len);

    //从通道读报文
    bool on_read(const char *pdata, int len, int flag);

    int duplextype_get(void)
    {
        return pio_node_->duplextype_get();
    }

    //此函数在发送结束后调用
    void send_status_end(int len);

    //电源控制
    bool power_on(void)
    {
        return power_ctrl('1');
    }
    bool power_off(void)
    {
        return power_ctrl('0');
    }
	bool power_ctrl(char value);

    io_node  *io_node_get(void)
    {
        return pio_node_;
    }
protected:

    int                 m_nLastSend; //最近周期内发送的报文字节数  未使用
    int                 m_nLastRcv;  //最近周期内收到的报文字节数  未使用
    int                 len_;

    bool                connected_;
    io_node             *pio_node_;
    channel             *pchannel_;
private:
};



















#endif




