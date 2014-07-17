#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <includes/includes.h>

using std::string;
using muduo::net::Buffer;

class protocol_runinfo {
public:
    uint32      m_nRcvPackTotal;       //接受数据包累计
    uint32      m_nSndPackTotal;       //发送数据包累计
    uint32      m_nRcvPackAverage;     //每秒接受数据包
    uint32      m_nSndPackAverage;     //每秒发送数据包

    uint32      m_nTimeOutPack;        //超时包累计
    uint32      m_nErrorPack;          //错误包累计
    uint32      m_nErrPackRate;        //误包率
};

class channel;
class protocol:boost::noncopyable{
public:
    protocol(const char *name = NULL):name_(name){}
    virtual ~protocol(){}

    //初始化规约
    virtual bool init();
    //反初始化
    virtual void uninit();

	void name_set(string &name){name_ = name;}
	string &name_get(void){return name_;}

	void describe_set(string &describe){describe_ = describe;}
	string &describe_get(void){return describe_;}

    //读取通道报文
    bool read_frchannel(const char *pdata, int len, int iflag = 0);

    //解析一帧报文
    virtual bool process_frame(const char * pdata, int len, int iflag = 0);

    int write_tochannel(const char *pdata, int len);

    //下行报文打包函数
    virtual int  package_aframe(char* pdata, int len);

    ////////////////////////////////////////////////////////////////////////////////
    //函数说明: 验证一帧是否有效
    //参数说明: BYTE* pData 报文数据缓冲区
    //参数说明: int iDataLen 缓冲区数据长度
    //参数说明: int& iPackLen 数据帧长度  iPackLen > 0:ok;  ==0:no end;  <0 error frame
    //返 回 值: virtual int 帧起始位置在数据缓冲区的偏移
    //备    注:
    ////////////////////////////////////////////////////////////////////////////////
    virtual int  validate_aframe(const char* pdata, int len, int& ipacklen);

    virtual bool handle_timer(void);

    //关连通道
    void channel_set(channel *pchannel)
    {
        pchannel_                   = pchannel;
    }

    static protocol *protocol_create(const char *name);

private:
    channel             *pchannel_;
    string              name_;
	string   			describe_;

	Buffer              buffer_;
	protocol_runinfo    runinfo_;
};















#endif

