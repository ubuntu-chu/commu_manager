#ifndef _NET_H_
#define _NET_H_

#include <includes/includes.h>
#include <protocol.h>
#include <protocol_raw.h>

enum{
    //含义为： 命令发出去后  等待后台对此命令的应答
    EVENT_ACK   = 0,
    //含义为： 只发送命令
    EVENT_CMD,
};

enum{
    RSP_OK      = 0,
    RSP_INVALID_CMD,
    RSP_INVALID_PARAM,
    RSP_EXEC_FAILURE,
    RSP_INVALID_PARAM_LEN,
    RSP_ABILITY_ERR,
    //主动模式下 收到应答
    RSP_ACK_IN_ACTIVE_MODE,
    //类型错误
    RSP_TYPE_ERR,
};

//function code
enum{
    def_FUNC_CODE_HEARBEAT  = 1,
    def_FUNC_CODE_READER_UPLOAD,
    def_FUNC_CODE_MODE_SET,
    def_FUNC_CODE_READER_SET,
    def_FUNC_CODE_RFID_R,
    def_FUNC_CODE_RFID_W,
    def_FUNC_CODE_SOUND_SET,
    def_FUNC_CODE_CHANNEL_POWER_SET,
    def_FUNC_CODE_CHANNEL_POWER_GET,
    def_FUNC_CODE_READER_QUERY,
};

class channel;

typedef boost::function<portBASE_TYPE (frame_ctl_t *pframe_ctl, uint8 *pbuf, uint16 len)> package_event_handler;

class CDevice_net:boost::noncopyable{
public:
    CDevice_net():max_wait_time_(3){};
    ~CDevice_net(){};

    int package_send(uint8 func_code, mac_frm_ctrl_t frm_ctl, char *pbuf, uint16 len);
    int package_send_sync(uint8 func_code, mac_frm_ctrl_t frm_ctl, char *pbuf, uint16 len);

    //发送rfid标签信息给后台
    int package_send_rfid(char *pbuf, uint16 len);
    //respond frame 发送应答
    int package_send_rsp(uint8 func_code, uint8 *prsp, uint16 len);
    //发送状态包
    int package_send_status(char *pbuf, uint16 len);
    //发送阅读器信息
    int package_send_readerinfo(char *pbuf, uint16 len);

    //获取是否有包事件  若有的话 则进行处理
    portBASE_TYPE package_event_fetch(void);
    //网络包事件 处理函数设置
    void package_event_handler_set(package_event_handler handler)
    {
        m_handler                       = handler;
    }

	void channel_set(channel *pchannel){pchannel_    = pchannel;}
private:
    CDevice_net(const CDevice_net &other);
    CDevice_net &operator =(const CDevice_net &other);

    int _package_send(uint8 func_code, mac_frm_ctrl_t frm_ctl, char *pbuf, uint16 len);
    int8 frm_ctrl_unpack(uint8_t* pbuf, uint16 len, frame_ctl_t *pfrm_ctl);
    void frm_ctl_init(frame_ctl_t *pfrm_ctl, mac_frm_ctrl_t frm_ctl, uint8 total, uint8 index, uint8 func_code, uint8 *pbuf, uint16 len);
    mac_frm_ctrl_t mac_frm_ctrl_init(uint8 ack, uint8 dir, uint8 ack_req, uint8 frm_type);

	channel                     *pchannel_;
    vector<char>                vec_send_;                  //发送容器
    vector<char>                vec_recv_;                  //接收容器
    vector<char>                *pvec_ret;
	int                         max_wait_time_;

	package_event_handler       m_handler;
};



#endif

