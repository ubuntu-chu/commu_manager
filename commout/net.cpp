#include "net.h"
#include <channel.h>
#include <utils.h>


void CDevice_net::frm_ctl_init(frame_ctl_t *pfrm_ctl, mac_frm_ctrl_t frm_ctl, uint8 total, uint8 index, uint8 func_code, uint8 *pbuf, uint16 len)
{
    protocol *pprotocol             = pchannel_->protocol_get();
    protocol_mac *pprotocol_mac    = reinterpret_cast<protocol_mac *>(pprotocol);

    pprotocol_mac->frm_ctl_init(pfrm_ctl, frm_ctl, total, index, func_code, pbuf, len);
}

mac_frm_ctrl_t CDevice_net::mac_frm_ctrl_init(uint8 ack, uint8 dir, uint8 ack_req, uint8 frm_type)
{
    protocol *pprotocol             = pchannel_->protocol_get();
    protocol_mac *pprotocol_mac    = reinterpret_cast<protocol_mac *>(pprotocol);

    return pprotocol_mac->mac_frm_ctrl_init(ack, dir, ack_req, frm_type);
}

int8 CDevice_net::frm_ctrl_unpack(uint8_t* pbuf, uint16 len, frame_ctl_t *pfrm_ctl)
{

    protocol *pprotocol             = pchannel_->protocol_get();
    protocol_mac *pprotocol_mac    = reinterpret_cast<protocol_mac *>(pprotocol);

    return pprotocol_mac->frm_ctrl_unpack(pbuf, len, pfrm_ctl);
}

int CDevice_net::_package_send(uint8 func_code, mac_frm_ctrl_t frm_ctl, char *pbuf, uint16 len)
{
    frame_ctl_t     t_frm_ctl;
    unsigned int  i;
    char          *pdata    = reinterpret_cast<char *>(&t_frm_ctl);

    vec_send_.clear();

    frm_ctl_init(&t_frm_ctl, frm_ctl, 1, 0, func_code, (uint8 *)pbuf, len);
    //组装数据
    for (i = 0; i < sizeof(frame_ctl_t); i++){
        vec_send_.push_back(*pdata++);
    }
    pdata                    = pbuf;
    //压入数据区
    for (i = 0; i < len; i++){
        vec_send_.push_back(*pdata++);
    }

    return 0;
}

int CDevice_net::package_send(uint8 func_code, mac_frm_ctrl_t frm_ctl, char *pbuf, uint16 len)
{
    _package_send(func_code, frm_ctl, pbuf, len);
    //调用通道写函数
    pchannel_->write_inloop(vec_send_);

    return 0;
}

int CDevice_net::package_send_sync(uint8 func_code, mac_frm_ctrl_t frm_ctl, char *pbuf, uint16 len)
{
    _package_send(func_code, frm_ctl, pbuf, len);
    //调用通道写函数
    return pchannel_->write_sync_inloop(vec_send_, max_wait_time_, &pvec_ret);
}

int CDevice_net::package_send_rfid(char *pbuf, uint16 len)
{
    return package_send(def_FUNC_CODE_RFID_R, mac_frm_ctrl_init(NO_ACK_FRAME, UPSTREAM, NO_ACK_REQUEST, FRAME_TYPE_DATA), pbuf, len);
}

//respond frame
int CDevice_net::package_send_rsp(uint8 func_code, uint8 *prsp, uint16 len)
{
    return package_send(func_code, mac_frm_ctrl_init(ACK_FRAME, UPSTREAM, NO_ACK_REQUEST, FRAME_TYPE_DATA), (char *)prsp, len);
}

int CDevice_net::package_send_status(char *pbuf, uint16 len)
{
    uint8   func_code           = def_FUNC_CODE_HEARBEAT;

    package_send(func_code, mac_frm_ctrl_init(NO_ACK_FRAME, UPSTREAM, ACK_REQUEST, FRAME_TYPE_DATA), (char *)pbuf, len);
    return 0;
}

int CDevice_net::package_send_readerinfo(char *pbuf, uint16 len)
{
    uint8   func_code           = def_FUNC_CODE_READER_QUERY;
    int     rt;

    rt = package_send_sync(func_code, mac_frm_ctrl_init(NO_ACK_FRAME, UPSTREAM, ACK_REQUEST, FRAME_TYPE_DATA), pbuf, len);

    //正确收到应答帧  判断是否为正确的应答帧
    if (0 == rt){
        frame_ctl_t         t_frame_ctl;

        utils::log_binary_buf("CDevice_net::package_send_readerinfo",
                reinterpret_cast<const char *>(&((*pvec_ret)[0])),
                pvec_ret->size());
        frm_ctrl_unpack(reinterpret_cast<uint8_t *>(&((*pvec_ret)[0])),
                pvec_ret->size(), &t_frame_ctl);
        //t_frame_ctl.data_ptr[0]  result
        if ((t_frame_ctl.data_ptr[0] != 0)
            || (t_frame_ctl.app_frm_ptr.fun != func_code)
            || (t_frame_ctl.mac_frm_ptr.ctl.ack_mask != 1)){
            rt                      = (portBASE_TYPE)-1;
            if (t_frame_ctl.data_ptr[0] != 0){
                LOG_WARN  << "Net recv:ack request not equal zero";
            }else if (t_frame_ctl.app_frm_ptr.fun != func_code) {
                LOG_WARN  << "Net recv:func code not equal";
            }else if (t_frame_ctl.mac_frm_ptr.ctl.ack_mask != 1) {
                LOG_WARN  << "Net recv:not ack frame";
            }
        }
    }else {
        if (rt < 0){
            LOG_WARN  << "Net recv:invalid ack frame";
        }
        rt                              = -1;
    }

    return rt;
}

portBASE_TYPE CDevice_net::package_event_fetch(void)
{
    if (NULL == pchannel_){
        return -1;
    }

    //接收到正确、完整帧
    if (enum_CH_FETCH_AFRAME == pchannel_->fetch(vec_recv_)){
        frame_ctl_t     t_frame_ctl;
        list_head_t     *device_maped_list_head  = pchannel_->device_maped_list_head_get();
        device_node     *pdevice_node = list_entry_offset(device_maped_list_head->m_next,
                                            class device_node, device_node::node_offset_get());

        frm_ctrl_unpack(reinterpret_cast<uint8_t *>(&*vec_recv_.begin()),
                vec_recv_.size(), &t_frame_ctl);

        if (t_frame_ctl.mac_frm_ptr.type != pdevice_node->class_type_get()){
            uint8   rsp_code            = RSP_TYPE_ERR;

            package_send_rsp(t_frame_ctl.app_frm_ptr.fun, &rsp_code, sizeof(rsp_code));
            LOG_ERROR  << "Net pakage:dev type error";
            return  (portBASE_TYPE)-1;
        }
        if (m_handler != NULL){
            m_handler(&t_frame_ctl, t_frame_ctl.data_ptr, t_frame_ctl.app_frm_ptr.len);
        }
    }

    return 0;

}

