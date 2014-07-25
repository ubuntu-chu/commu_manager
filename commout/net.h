#ifndef _NET_H_
#define _NET_H_

#include <includes/includes.h>
#include <protocol.h>
#include <protocol_raw.h>

enum{
    EVENT_ACK   = 0,
    EVENT_CMD,
};

enum{
    RSP_OK      = 0,
    RSP_INVALID_CMD,
    RSP_INVALID_PARAM,
    RSP_EXEC_FAILURE,
    RSP_INVALID_PARAM_LEN,
    RSP_ABILITY_ERR,
    //涓诲姩妯″紡涓?鏀跺埌搴旂瓟甯?
    RSP_ACK_IN_ACTIVE_MODE,
    RSP_TYPE_ERR,
};

//function code
enum{
    def_FUNC_CODE_HEARBEAT  = 1,
    def_FUNC_CODE_READER_QUERY,
    def_FUNC_CODE_MODE_SET,
    def_FUNC_CODE_READER_SET,
    def_FUNC_CODE_RFID_R,
    def_FUNC_CODE_RFID_W,
};

class channel;

class CDevice_net:boost::noncopyable{
public:
    CDevice_net(){};
    ~CDevice_net(){};

    void frm_ctl_init(frame_ctl_t *pfrm_ctl, mac_frm_ctrl_t frm_ctl, uint8 total, uint8 index, uint8 func_code, uint8 *pbuf, uint16 len);
    mac_frm_ctrl_t mac_frm_ctrl_init(uint8 ack, uint8 dir, uint8 ack_req, uint8 frm_type);

    int package_send(uint8 func_code, mac_frm_ctrl_t frm_ctl, char *pbuf, uint16 len);
    int package_send_rfid(char *pbuf, uint16 len);
    //respond frame
    int package_send_rsp(uint8 func_code, uint8 *prsp, uint16 len);
    int package_send_status(char *pbuf, uint16 len);
    int package_send_readerinfo(char *pbuf, uint16 len);

	void channel_set(channel *pchannel){pchannel_    = pchannel;}
private:
    CDevice_net(const CDevice_net &other);
    CDevice_net &operator =(const CDevice_net &other);

	channel                     *pchannel_;
};




















#endif

