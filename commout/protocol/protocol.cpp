#include "protocol.h"
#include <protocol_rfid.h>
#include <protocol_raw.h>
#include <channel.h>


//初始化规约
bool protocol::init()
{

    return true;
}

//反初始化
void protocol::uninit()
{

}

//读取通道报文
bool protocol::read_frchannel(const char *pdata, int len, int iflag)
{
    //通道信息
    if(iflag != 0) {
        process_frame(pdata, len, iflag);
        return true;
    }
    buffer_.append(pdata, len);
    while (buffer_.readableBytes()){
        int         packlen;
        int         rt;
        const char  *paddr        = buffer_.peek();

        rt = validate_aframe(paddr, buffer_.readableBytes(), packlen);
        //打印帧
        //帧尚未接收完全
        if (rt == 0){
            break;
        //错误帧
        }else if (rt < 0){
            buffer_.retrieve(packlen);
            runinfo_.m_nErrorPack++;
            break;
        //正确帧
        }else {
            runinfo_.m_nRcvPackTotal++;
        }
        process_frame(paddr, packlen, iflag);
        buffer_.retrieve(packlen);
    }

    return true;
}

//解析一帧报文
bool protocol::process_frame(const char * pdata, int len, int iflag)
{

    return true;
}

int protocol::write_tochannel(const char *pdata, int len)
{
    if (NULL == pchannel_){
        return CHANNEL_NOTINIT;
    }
    return pchannel_->write(pdata, len);
}

int  protocol::package_aframe(char* pdata, int len)
{

    return 0;
}

int  protocol::validate_aframe(const char* pdata, int len, int& ipacklen)
{

    return 0;
}

bool protocol::handle_timer(void)
{
    return true;
}

protocol *protocol::protocol_create(const char *name)
{
    protocol *pprotocol = NULL;

    LOG_INFO << "protocol[" << name << "] will be created!";
    if (0 == strcmp("rfid", name)){
        pprotocol                   = new protocol_rfid();
    }else if (0 == strcmp("raw", name)){
        pprotocol                   = new protocol_raw();
    }

    if (pprotocol == NULL){
        LOG_ERROR << "protocol[" << name << "] can not find!";
    }

    return pprotocol;
}




