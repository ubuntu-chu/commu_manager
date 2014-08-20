#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <includes/includes.h>
#include "rfid.h"

//阅读器能力:读取EPC
#define         TERMINAL_ABILITY_NONE               (0x00)
//阅读器能力:读取EPC+DATA
#define         TERMINAL_ABILITY_R_DATA             (0x01)
//阅读器能力:写DATA
#define         TERMINAL_ABILITY_W_DATA             (0x02)
//阅读器能力:未使用
#define         TERMINAL_ABILITY_W_EPC              (0x04)

enum{
    //主动模式
    MODE_INITIATIVE     = 0,
    //被动模式
    MODE_PASSIVITY,
};

struct app_rfidinfo{
    uint8               m_epclen;
    //fix 12bytes
    uint8               m_epcarray[12];
    //fixed start  0xfe
    uint8               m_initseq_hi;           //从此向下为数据区：总共12个字节  如何使用取决与应用程序
    //0xef
    uint8               m_initseq_low;
    //attr
    uint8               m_attr_hi;
    uint8               m_attr_low;
    //storage
    uint8               m_storage_hi;
    uint8               m_storage_low;
    //empty weight
    uint8               m_empty_weight_hi;
    uint8               m_empty_weight_low;
    //total weight
    uint8               m_total_weight_hi;
    uint8               m_total_weight_low;
    //reserved
    uint8               m_reserved[2];
};

enum {
    enum_APP_STATUS_INIT = 0,
    enum_APP_STATUS_INIT_ERR,
    enum_APP_STATUS_SEND_READERINFO,
    enum_APP_STATUS_RUN,
    enum_APP_STATUS_EXIT,
};

struct _app_runinfo_{
    CDevice_Rfid                        *m_pdevice_rfid;                //rfid设备指针
    CDevice_net                         *m_pdevice_net;                 //net设置指针

    int                                 timer_fd_;                      //心跳定时器设备fd
    pid_t                               ppid_;                          //父进程pid

    struct epc_info                     m_epcinfo;
    struct app_rfidinfo                 m_rfidinfo;
    uint8                               m_ability;
    uint8                               m_mode;
    volatile sig_atomic_t              m_status;
    uint8                               m_reader_numbs;    //通道下所挂接设备数量

    boost::ptr_vector<channel>          channel_vector_;                //通道容器

};
typedef struct _app_runinfo_ app_runinfo_t;

class CApplication{
public:
    CApplication(){};
    ~CApplication(){};

    static CApplication *GetInstance(void);
    portBASE_TYPE run(void);
    portBASE_TYPE init(const char *log_file_path, const char *config_file_path);
    void quit(void);

private:
    CApplication(const CApplication &other);
    CApplication &operator =(const CApplication &other);

    void content_readerinfo_make(uint8 *pbuf, uint16 *plen);
    uint8 protocol_rfid_write(uint8 *pbuf, uint16 len);
    portBASE_TYPE readerrfid_init(void);
    portBASE_TYPE readerrfid_query(uint8 *pbuf, uint16 *plen);
    portBASE_TYPE readerrfid_write(uint8 *pbuf, uint16 *plen);
    portBASE_TYPE readerrfid_sound_set(uint8 *pbuf, uint16 *plen);
    portBASE_TYPE readerrfid_channelpower_set(uint8 *pbuf, uint16 *plen);
    portBASE_TYPE readerrfid_channelpower_get(uint8 *pbuf, uint16 *plen);
    portBASE_TYPE containerrfid_r_epc(CDevice_Rfid  *pdevice_rfid);
    portBASE_TYPE containerrfid_w_epc(CDevice_Rfid  *pdevice_rfid, uint8 *pepc);
    portBASE_TYPE containerrfid_r_data(CDevice_Rfid *pdevice_rfid, uint8 index, uint8 *pbuff, uint16 *plen, uint8 ctrl);
    portBASE_TYPE containerrfid_w_data(CDevice_Rfid *pdevice_rfid, uint8 epc_len, uint8 *pepc, uint8 *pdata);
    portBASE_TYPE device_status_send(void);

    portBASE_TYPE package_event_handler(frame_ctl_t *pframe_ctl, uint8 *pbuf, uint16 len);

    portBASE_TYPE protocol_rfid_read(void);

    bool timer_timeout_occured(void);

    static CApplication     *m_pcapplicaiton;
    struct _app_runinfo_    m_app_runinfo;
};

struct process_stat   *process_stat_ptr_get(void);
void process_stat_set(int stat);

#endif
