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
        process_aframe(pdata, len, iflag);
        return true;
    }
    inbuffer_.append(pdata, len);
    while (inbuffer_.readableBytes()){
        int         packlen;
        int         rt;
        const char  *paddr        = inbuffer_.peek();
        int         len            = inbuffer_.readableBytes();

        utils::log_binary_buf("protocol::read_frchannel", paddr, len);
        rt = validate_aframe(paddr, len, packlen);
        //打印帧
        //帧尚未接收完全
        if (rt == 0){
            break;
        //错误帧
        }else if (rt < 0){
            //让应用软件处理错误帧
            process_aframe(paddr, packlen, rt);
            inbuffer_.retrieve(packlen);
            runinfo_.m_nErrorPack++;
            break;
        //正确帧
        }else {
            runinfo_.m_nRcvPackTotal++;
        }
        process_aframe(paddr, packlen, iflag);
        inbuffer_.retrieve(packlen);
    }

    return true;
}

//解析一帧报文
bool protocol::process_aframe(const char * pdata, int len, int iflag)
{
    if (NULL == pchannel_){
        return false;
    }
    pchannel_->on_process_aframe(pdata, len, iflag);
    return true;
}

int protocol::write_tochannel(const char *pdata, int len)
{
    if (NULL == pchannel_){
        return CHANNEL_NOTINIT;
    }
    outbuffer_.retrieveAll();
    if (enum_WORK_TYPE_HALF_DUPLEX == pchannel_->duplextype_get()){
        inbuffer_.retrieveAll();
    }
    package_aframe(const_cast<char *>(pdata), len);
    return pchannel_->on_write(outbuffer_.peek(), outbuffer_.readableBytes());
}

int  protocol::package_aframe(char* pdata, int len)
{
    LOG_TRACE;

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
    if (0 == strcmp(def_PROTOCOL_RFID_NAME, name)){
        pprotocol                   = new protocol_rfid();
    }else if (0 == strcmp(def_PROTOCOL_MAC_NAME, name)){
        pprotocol                   = new protocol_mac();
    }

    if (pprotocol == NULL){
        LOG_ERROR << "protocol[" << name << "] can not find!";
    }

    return pprotocol;
}




