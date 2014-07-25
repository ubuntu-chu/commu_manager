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

class protocol_info{
public:
    protocol_info(){}
    ~protocol_info(){}
public:
    void package_addr_set(char *package){m_ppackage     = package;}
    char *package_addr_get(void){return m_ppackage;}

    void package_len_set(uint16 len){m_len  = len;}
    uint16 package_len_get(void){return m_len;}

protected:
    char               *m_ppackage;
    uint16              m_len;
};

enum protocol_phase{
    enum_PROTOCOL_PREPARE   = 0,
    enum_PROTOCOL_DONE,
};

class channel;
class protocol:boost::noncopyable{
public:
//    typedef boost::function<bool (enum protocol_phase phase, boost::shared_ptr<protocol_info>&info)> process_aframe_cb;

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
    virtual bool process_aframe(const char * pdata, int len, int iflag = 0);

    int write_tochannel(const char *pdata, int len);

    //下行报文打包函数  目前不关心其返回值  程序可返回其打包后的整体长度
    virtual int  package_aframe(char* pdata, int len);

    ////////////////////////////////////////////////////////////////////////////////
    //函数说明: 验证一帧是否有效
    //参数说明: BYTE* pData 报文数据缓冲区
    //参数说明: int iDataLen 缓冲区数据长度
    //参数说明: int& iPackLen 数据帧长度  当数据仍在接收时 不关心ipacklen的值
    //返 回 值: < 0 数据仍在接受   = 0 一帧接收完成   < 0 一帧接收出错
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

protected:
	Buffer              inbuffer_;
	Buffer              outbuffer_;
    channel             *pchannel_;

private:
    string              name_;
	string   			describe_;

    boost::shared_ptr<protocol_info> info_;
	protocol_runinfo    runinfo_;
};















#endif

