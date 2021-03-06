#ifndef _IO_BASE_H
#define _IO_BASE_H

#include <includes/includes.h>
#include <includes/includes.h>
#include "io_node.h"

class channel;
//通信基类
class io_base{
public:
    io_base(const io_node *pio_node = NULL):pio_node_(pio_node){}
    virtual ~io_base(){}

    //初始化通信介质
    virtual bool init(void);

    //反初始化
    virtual bool uninit();

    //关连导出类
    void channel_set(channel *pchannel)
    {
        pchannel_                   = pchannel;
    }

    //连接通信介质
    virtual bool connect(bool brelink = true);

    //断开通信介质，
    virtual bool disconnect(void);

    //报文拆帧后发送
    virtual int send_package(char *pdata, size_t len);

    //向通道写报文
    virtual int send_data(char *pdata, size_t len);

    //从通道读报文
    bool on_read(const char *pdata, size_t len, int flag);

protected:

    int                 m_nLastSend; //最近周期内发送的报文字节数
    int                 m_nLastRcv;  //最近周期内收到的报文字节数

    const io_node      *pio_node_;
    channel             *pchannel_;
private:
};



















#endif




