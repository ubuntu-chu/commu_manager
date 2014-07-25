#ifndef _APPLICATION_H
#define _APPLICATION_H

#include "rfid.h"

#define         TERMINAL_ABILITY_R_DATA             (0x01)
#define         TERMINAL_ABILITY_W_DATA             (0x02)
#define         TERMINAL_ABILITY_W_EPC              (0x04)

enum{
    MODE_INITIATIVE     = 0,
    MODE_PASSIVITY,
};

struct app_rfidinfo{
    uint8               m_epclen;
    //fix 12bytes
    uint8               m_epcarray[12];
    //fixed start  0xfe
    uint8               m_initseq_hi;
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

struct _app_runinfo_{
    CDevice_Rfid       *m_pdevice_rfid;
    CDevice_net        *m_pdevice_net;

    struct reader_info  *m_readerinfo;
    struct epc_info     m_epcinfo;
    struct app_rfidinfo m_rfidinfo;
    uint8               m_ability;
    uint8               m_mode;
    uint8               m_status;
    uint8               m_reader_numbs;    //通道下所挂接设备数量
};
typedef struct _app_runinfo_ app_runinfo_t;

class CApplication{
public:
    static CApplication *GetInstance(void);
    portBASE_TYPE run(void);
    portBASE_TYPE init(const char *config_file_path);

private:
    CApplication(){};
    ~CApplication(){};
    CApplication(const CApplication &other);
    CApplication &operator =(const CApplication &other);

    void content_readerinfo_make(uint8 *pbuf, uint16 *plen);
    uint8 protocol_rfid_write(uint8 *pbuf, uint16 len);
    portBASE_TYPE readerrfid_init(void);
    portBASE_TYPE readerrfid_write(uint8 *pbuf, uint16 *plen);
    portBASE_TYPE containerrfid_r_epc(CDevice_Rfid  *pdevice_rfid);
    portBASE_TYPE containerrfid_w_epc(CDevice_Rfid  *pdevice_rfid, uint8 *pepc);
    portBASE_TYPE containerrfid_r_data(CDevice_Rfid *pdevice_rfid, uint8 index, uint8 *pbuff, uint16 *plen, uint8 ctrl);
    portBASE_TYPE containerrfid_w_data(CDevice_Rfid *pdevice_rfid, uint8 epc_len, uint8 *pepc, uint8 *pdata);
    portBASE_TYPE device_status_send(void);

    portBASE_TYPE protocol_rfid_read(void);


    static CApplication     *m_pcapplicaiton;
    struct _app_runinfo_    m_app_runinfo;
};

#endif
