#include "protocol_raw.h"
#include <io_base.h>
#include <channel.h>
#include <device_node.h>

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
    protocol::init();
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
//    frame_ctl_t *pframe_ctl     = reinterpret_cast<frame_ctl_t *>(pdata);
    frame_ctl_t t_frame_ctl;
    frame_ctl_t *pframe_ctl     = &t_frame_ctl;
    uint8 ptr[1000];
    uint16_t val;
    uint16_t val_len = 0x00;
    uint8_t *tmp_ptr = ptr;
    mac_frame_t* mac_frm_ptr        = &(pframe_ctl->mac_frm_ptr);
    app_frame_t* app_frm_ptr        = &(pframe_ctl->app_frm_ptr);
//    uint8_t* pData                  = pframe_ctl->data_ptr;
    uint8_t* pData;

    //可能会存在pdata所执行内存字节对齐问题
    memcpy(reinterpret_cast<void *>(&t_frame_ctl), reinterpret_cast<const void *>(pdata),
            sizeof(frame_ctl_t));

    if (app_frm_ptr->len > 0x00) {

        //重新指定pData地址
        pData                       = reinterpret_cast<uint8 *>(pdata + sizeof(frame_ctl_t));

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
        //数据区为空的时候  不包含包索引、数据长度、数据区
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
//返 回 值: = 0 数据仍在接受   > 0 一帧接收完成   < 0 一帧接收出错
//备    注:
////////////////////////////////////////////////////////////////////////////////
#define     def_FRAME_HEAD_FRAME_LEN_SUM            (4)

int  protocol_mac::validate_aframe(struct validate_aframe_info *pinfo, int& ipacklen)
{
    uint16 chk_sum;
    uint16 frame_len;
    int     rt  = 1;
    const char *pdata                  = pinfo->pdata_;
    int     len                         = pinfo->len_;
    int     i   = 0;


    ipacklen                            = 0;
    //数据帧长度 不能小于4
    if (len < def_FRAME_HEAD_FRAME_LEN_SUM){
        rt                              = 0;
        goto quit;
    }
    //找到帧头  若未找到帧头 则认为此帧为错误帧 丢弃
    for (i = 0; i < len - 1; i++){
        //check head
        if ((static_cast<uint8>(pdata[i]) == static_cast<uint8>(def_FRAME_delimiter & 0x00ff))
            && (static_cast<uint8>(pdata[i+1]) == static_cast<uint8>(def_FRAME_delimiter >> 8))) {
            //记录下实际帧头位置
            pinfo->pdata_actual_        = &pdata[i];
            break;
        }
    }
    //在整个数据中 未找到帧头 丢弃此帧
    if (i == len - 1){
        ipacklen                        = len;
        rt                              = -1;
        goto quit;
    }

    //更新帧头位置
    pdata                               = pinfo->pdata_actual_;
    //查看是否完整的接收到一帧数据
    frame_len                           = LD_DWORD(&pdata[2]);
    //check len
    if (frame_len > (len - def_FRAME_HEAD_FRAME_LEN_SUM - i)) {
        rt                              = 0;
        goto quit;
    }

    //下面的情况 认为帧已经接收完全 需要对帧错误情况进行判断
    ipacklen                            = frame_len + i + def_FRAME_HEAD_FRAME_LEN_SUM;
    //check tail
    if ((static_cast<uint8>(pdata[frame_len + 2]) != static_cast<uint8>(def_FRAME_END_delimiter & 0x00ff))
        || (static_cast<uint8>(pdata[frame_len + 3]) != static_cast<uint8>(def_FRAME_END_delimiter >> 8))) {
        rt                              = -2;
        goto quit;
    }

    //check sum
    chk_sum = LD_WORD(&pdata[frame_len]);
    if (chk_sum != frm_ck_sum(
            reinterpret_cast<uint8_t *>(const_cast<char *>(pdata)), frame_len)) {
        rt                              = -3;
        goto quit;
    }

quit:
    LOG_INFO << "return value = " << rt << " ipacklen = " << ipacklen << " len = " << len;

    return rt;
}

bool protocol_mac::handle_timer(void)
{
    LOG_DEBUG;
    return true;
}

void protocol_mac::frm_ctl_init(frame_ctl_t *pfrm_ctl, mac_frm_ctrl_t frm_ctl, uint8 total, uint8 index, uint8 func_code, uint8 *pbuf, uint16 len)
{
    io_base *pio_base           = pchannel_->io_base_get();
    io_node *pio_node           = pio_base->io_node_get();
    io_node *pio_node_map       = pio_node->io_node_map_get();

    io_com_ext_node *pio_com_ext_node =
            reinterpret_cast<io_com_ext_node *>(const_cast<io_node *>(pio_node_map));
    device_node  *pdevice_node;

    //获取此通道下io下所挂接设备
    //同一个io下所有设备的class type必须相同
    list_head_t * plist_head  = pio_com_ext_node->device_list_head_get();

    pdevice_node    = list_entry_offset(plist_head->m_next, class device_node, device_node::node_offset_get());

    memset(pfrm_ctl, 0, sizeof(frame_ctl_t));

    pfrm_ctl->mac_frm_ptr.delimiter_start = def_FRAME_delimiter;
    pfrm_ctl->mac_frm_ptr.seq_id = seq_id_get();

    pfrm_ctl->mac_frm_ptr.ctl       = frm_ctl;

    pfrm_ctl->mac_frm_ptr.time = time(NULL);

    pfrm_ctl->mac_frm_ptr.dev_adr = pio_com_ext_node->device_addr_get();
    pfrm_ctl->mac_frm_ptr.sen_adr = pio_com_ext_node->sensor_addr_get();

    pfrm_ctl->mac_frm_ptr.type  = pdevice_node->class_type_get();
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


int8 protocol_mac::frm_ctrl_unpack(uint8_t* pbuf, uint16 len, frame_ctl_t *pfrm_ctl)
{
    //valid frame   for safe: byte by byte
    pfrm_ctl->mac_frm_ptr.delimiter_start       = LD_DWORD(&pbuf[0]);
    //不包括开始界定符 和 帧长
    pfrm_ctl->mac_frm_ptr.len           = len - 4;
    pfrm_ctl->mac_frm_ptr.seq_id        = LD_DWORD(&pbuf[4]);
    pfrm_ctl->mac_frm_ptr.dev_adr       = LD_DWORD(&pbuf[12]);
    pfrm_ctl->mac_frm_ptr.sen_adr       = LD_DWORD(&pbuf[16]);
    pfrm_ctl->mac_frm_ptr.time      = LD_DWORD(&pbuf[8]);
    //set sys time
//    cpu_sys_time_set(pfrm_ctl->mac_frm_ptr.time);
    memcpy(&pfrm_ctl->mac_frm_ptr.ctl, &pbuf[6], 2);
    //pfrm_ctl->mac_frm_ptr.ctl     = (mac_frm_ctrl_t)LD_WORD(&pbuf[6]);
    pfrm_ctl->mac_frm_ptr.type      = pbuf[20];
    pfrm_ctl->app_frm_ptr.fun       = pbuf[21];
    pfrm_ctl->app_frm_ptr.sum       = pbuf[22];
    pfrm_ctl->app_frm_ptr.idex      = pbuf[23];
    pfrm_ctl->app_frm_ptr.len       = LD_WORD(&pbuf[24]);
    pfrm_ctl->data_ptr              = &pbuf[26];

    return 0;
}


