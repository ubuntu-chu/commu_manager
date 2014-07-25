#include "protocol_raw.h"

uint16_t          seq_id = 0;

uint16_t seq_id_get(void)
{
    return seq_id++;
}

//check sum
uint16_t frm_ck_sum(uint8_t * ptr, uint16_t len) {
    uint16_t val = 0x0000;

    for (uint16_t i = 0x0000; i < len; i++) {
        val += ptr[i];
    }

    return val;
}

//初始化规约
bool protocol_mac::init()
{
    seq_id                          = 0;
    return true;
}

//反初始化
void protocol_mac::uninit()
{

}

//打包函数
int  protocol_mac::package_aframe(char* pdata, int len)
{
    frame_ctl_t *pframe_ctl     = reinterpret_cast<frame_ctl_t *>(pdata);
    uint8 ptr[1000];
    uint16_t val;
    uint16_t val_len = 0x00;
    uint8_t *tmp_ptr = ptr;
    mac_frame_t* mac_frm_ptr        = &(pframe_ctl->mac_frm_ptr);
    app_frame_t* app_frm_ptr        = &(pframe_ctl->app_frm_ptr);
    uint8_t* pData                  = pframe_ctl->data_ptr;

    if (app_frm_ptr->len > 0x00) {

        mac_frm_ptr->len = def_FRAME_MAC_LEN + def_FRAME_APP_HEAD_LEN
                + def_FRAME_PACK_IDEX_LEN         // len assign
                + def_FRAME_DATA_LEN + app_frm_ptr->len + def_FRAME_CK_LEN
                + def_FRAME_DELIMITER_LEN;

        val_len = def_FRAME_DELIMITER_LEN + def_FRAME_LEN_LEN
                + def_FRAME_MAC_LEN;
        memcpy(tmp_ptr, mac_frm_ptr, val_len);
        tmp_ptr = tmp_ptr + val_len;

        val_len = def_FRAME_APP_HEAD_LEN + def_FRAME_PACK_IDEX_LEN;
        memcpy(tmp_ptr, app_frm_ptr, val_len);
        tmp_ptr = tmp_ptr + val_len;

        val_len = def_FRAME_DATA_LEN;
        memcpy(tmp_ptr, (void *) &app_frm_ptr->len, val_len);
        tmp_ptr = tmp_ptr + val_len;

        val_len = app_frm_ptr->len;
        memcpy(tmp_ptr, pData, val_len);
        tmp_ptr = tmp_ptr + val_len;

        val_len = def_FRAME_DELIMITER_LEN + def_FRAME_LEN_LEN
                + def_FRAME_MAC_LEN + def_FRAME_APP_HEAD_LEN
                + def_FRAME_PACK_IDEX_LEN + def_FRAME_DATA_LEN
                + app_frm_ptr->len;

    } else {
        mac_frm_ptr->len = def_FRAME_MAC_LEN + def_FRAME_APP_HEAD_LEN
                + def_FRAME_CK_LEN + def_FRAME_DELIMITER_LEN;

        val_len = def_FRAME_DELIMITER_LEN + def_FRAME_LEN_LEN
                + def_FRAME_MAC_LEN;
        memcpy(tmp_ptr, mac_frm_ptr, val_len);
        tmp_ptr = tmp_ptr + val_len;

        val_len = def_FRAME_FUN_LEN + def_FRAME_SUM_PACK_LEN;
        memcpy(tmp_ptr, app_frm_ptr, val_len);
        tmp_ptr = tmp_ptr + val_len;

        val_len = def_FRAME_DELIMITER_LEN + def_FRAME_LEN_LEN
                + def_FRAME_MAC_LEN + def_FRAME_APP_HEAD_LEN;
    }

    val = frm_ck_sum(ptr, val_len);                                    // cal CK

    ST_WORD(tmp_ptr, val);
    tmp_ptr = tmp_ptr + def_FRAME_CK_LEN;

    ST_WORD(tmp_ptr, def_FRAME_END_delimiter);

    val_len = val_len + def_FRAME_CK_LEN + def_FRAME_DELIMITER_LEN; // whole frame len

    outbuffer_.append(ptr, val_len);

    return val_len;                                           // whole frame len
}

bool protocol_mac::process_aframe(const char * pdata, int len, int iflag)
{
    protocol::process_aframe(pdata, len, iflag);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//函数说明: 验证一帧是否有效
//参数说明: BYTE* pData 报文数据缓冲区
//参数说明: int iDataLen 缓冲区数据长度
//参数说明: int& iPackLen 数据帧长度  当数据仍在接收时 不关心ipacklen的值
//返 回 值: < 0 数据仍在接受   = 0 一帧接收完成   < 0 一帧接收出错
//备    注:
////////////////////////////////////////////////////////////////////////////////

int  protocol_mac::validate_aframe(const char* pdata, int len, int& ipacklen)
{
    uint16 chk_sum;
    uint16 frame_len;
    int     rt  = 1;

    ipacklen                    = 0;
    LOG_INFO << "called with len = " << len;
    //数据帧长度 不能小于4
    if (len < 4){
        rt                      = 0;
        goto quit;
    }
    frame_len = LD_DWORD(&pdata[2]);
    //check len
    if (frame_len != len - 4) {
        rt                      = 0;
        goto quit;
    }

    //下面的情况 认为帧已经接收完全 需要对帧错误情况进行判断
    ipacklen                    = len;
    //check head
    if ((pdata[0] != (def_FRAME_delimiter & 0x00ff))
            && (pdata[1] != (def_FRAME_delimiter >> 8))) {
        rt                      = -1;
        goto quit;
    }
    //check tail
    if ((pdata[len - 2] != (def_FRAME_END_delimiter & 0x00ff))
            && (pdata[len - 1] != (def_FRAME_END_delimiter >> 8))) {
        rt                      = -2;
        goto quit;
    }
#if 0
    //check sum
    chk_sum = LD_WORD(&pdata[len-4]);
    if (chk_sum != frm_ck_sum(
            reinterpret_cast<uint8_t *>(const_cast<char *>(pdata)), len - 4)) {
        rt                      = -3;
        goto quit;
    }
#endif

quit:
    LOG_INFO << "return value = " << rt << " ipacklen = " << ipacklen;
    return rt;
}

bool protocol_mac::handle_timer(void)
{
    LOG_DEBUG;
    return true;
}

void protocol_mac::frm_ctl_init(frame_ctl_t *pfrm_ctl, mac_frm_ctrl_t frm_ctl, uint8 total, uint8 index, uint8 func_code, uint8 *pbuf, uint16 len)
{
    memset(pfrm_ctl, 0, sizeof(frame_ctl_t));

    pfrm_ctl->mac_frm_ptr.delimiter_start = def_FRAME_delimiter;
    pfrm_ctl->mac_frm_ptr.seq_id = seq_id_get();

    pfrm_ctl->mac_frm_ptr.ctl       = frm_ctl;

    pfrm_ctl->mac_frm_ptr.time = 0;

    pfrm_ctl->mac_frm_ptr.dev_adr = 0;
    pfrm_ctl->mac_frm_ptr.sen_adr = 0;

    pfrm_ctl->mac_frm_ptr.type  = 0;
    pfrm_ctl->app_frm_ptr.fun   = func_code;

    pfrm_ctl->app_frm_ptr.sum   = total;
    pfrm_ctl->app_frm_ptr.idex  = index;

    pfrm_ctl->app_frm_ptr.len   = len;
    pfrm_ctl->data_ptr          = (uint8_t *)pbuf;
}

mac_frm_ctrl_t protocol_mac::mac_frm_ctrl_init(uint8 ack, uint8 dir, uint8 ack_req, uint8 frm_type)
{
    mac_frm_ctrl_t t_frm_ctrl;

    memset(&t_frm_ctrl, 0, sizeof(mac_frm_ctrl_t));
    t_frm_ctrl.ack_mask         = ack;
    t_frm_ctrl.direction        = dir;
    t_frm_ctrl.ack_req          = ack_req;
    t_frm_ctrl.frm_type         = frm_type;

    return t_frm_ctrl;
}



int protocol_mac::package_send(uint8 func_code, mac_frm_ctrl_t frm_ctl, char *pbuf, uint16 len)
{
    frame_ctl_t t_frm_ctl;

    frm_ctl_init(&t_frm_ctl, frm_ctl, 1, 0, func_code, (uint8 *)pbuf, len);
    write_tochannel((char *)&t_frm_ctl, sizeof(frame_ctl_t));

    return 0;
}
