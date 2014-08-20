#include "protocol.h"
#include <protocol_rfid.h>
#include <protocol_raw.h>
#include <channel.h>
#include <utils.h>


//初始化规约
bool protocol::init()
{
    inbuffer_.retrieveAll();
    outbuffer_.retrieveAll();

    return true;
}

//反初始化
void protocol::uninit()
{

}

//读取通道报文
bool protocol::read_frchannel(const char *pdata, int len, int iflag)
{
    struct validate_aframe_info     t_validate_aframe_info;
    char   log_msg_head[100];

    //通道信息  iflag!=0 代表错误数据帧
    if(iflag != 0) {
        process_aframe(pdata, len, iflag);

        return true;
    }
    //拼接日志信息
    strcpy(log_msg_head, pchannel_->io_node_name_get());
    strcat(log_msg_head, "-> protocol::read_frchannel");

    //将接收到的数据 追加到inbuffer中
    inbuffer_.append(pdata, len);
    //循环处理inbuffer中的数据
    while (inbuffer_.readableBytes()){
        int         packlen;
        int         rt;
        //获取数据首地址
        const char  *paddr        = inbuffer_.peek();
        //获取数据数量
        int         no             = inbuffer_.readableBytes();

        utils::log_binary_buf(log_msg_head, paddr, no);
        //使用pdata_  pdata_actual_ 原因： 整个数据中 可能在完整的一帧前 包含有些无效数据 其中pdata_
        //指向整个数据的起始地址 pdata_actual_指向完整一帧的数据起始地址 pdata_actual_和pdata_之间的
        //差值即为完整一帧之前的无效数据个数
        t_validate_aframe_info.pdata_                       = paddr;
        t_validate_aframe_info.pdata_actual_                = t_validate_aframe_info.pdata_;
        t_validate_aframe_info.len_                         = no;
        //packlen的值代表从整个数据的起始地址到完整一帧的尾部数据地址的长度
        rt = validate_aframe(&t_validate_aframe_info, packlen);
        //pdata_actual_为实际的数据帧开始地址
        paddr                                               = t_validate_aframe_info.pdata_actual_;
        //计算帧实际长度
        packlen                                             -= t_validate_aframe_info.pdata_-paddr;

        //帧尚未接收完全
        if (rt == 0){
            break;
        //错误帧
        }else if (rt < 0){
            //让应用软件处理错误帧
            process_aframe(paddr, packlen, rt);
            //将错误帧数据从inbuffer中去除
            inbuffer_.retrieve(packlen);
            runinfo_.m_nErrorPack++;
            continue;
        //正确帧
        }else {
            runinfo_.m_nRcvPackTotal++;
        }
        //处理帧数据
        process_aframe(paddr, packlen, iflag);
        //将帧数据从inbuffer中去除
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
    //将地址为pdata 长度为len的数据 进行协议打包   打包后的数据存入outbuffer中
    package_aframe(const_cast<char *>(pdata), len);
    return pchannel_->on_write(outbuffer_.peek(), outbuffer_.readableBytes());
}

//帧数据打包
int  protocol::package_aframe(char* pdata, int len)
{
    LOG_TRACE;

    return 0;
}

//验证一帧是否完整
int  protocol::validate_aframe(struct validate_aframe_info *pinfo, int& ipacklen)
{

    return 0;
}

//未使用
bool protocol::handle_timer(void)
{
    return true;
}

//协议创建
protocol *protocol::protocol_create(const char *name)
{
    protocol *pprotocol = NULL;

    LOG_TRACE << "protocol[" << name << "] will be created!";
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




