#include "net.h"
#include <channel.h>


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


int CDevice_net::package_send(uint8 func_code, mac_frm_ctrl_t frm_ctl, char *pbuf, uint16 len)
{
    frame_ctl_t     t_frm_ctl;
    vector<char>   vec_send;
    unsigned int  i;
    char          *pdata    = reinterpret_cast<char *>(&t_frm_ctl);

    frm_ctl_init(&t_frm_ctl, frm_ctl, 1, 0, func_code, (uint8 *)pbuf, len);
    //组装数据
    for (i = 0; i < sizeof(frame_ctl_t); i++){
        vec_send.push_back(*pdata++);
    }
    pdata                    = pbuf;
    //压入数据区
    for (i = 0; i < len; i++){
        vec_send.push_back(*pdata++);
    }

    //调用通道写函数
    pchannel_->write_inloop(vec_send);

    return 0;
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

    package_send(func_code, mac_frm_ctrl_init(NO_ACK_FRAME, UPSTREAM, ACK_REQUEST, FRAME_TYPE_DATA), pbuf, len);
    //wait for answer
//        return package_recv_handle(EVENT_ACK, func_code);

    return 0;
}


