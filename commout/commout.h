#ifndef _COMMOUT_H
#define _COMMOUT_H

#include "rfid.h"

#define         TERMINAL_ABILITY_R_DATA             (0x01)
#define         TERMINAL_ABILITY_W_DATA             (0x02)
#define         TERMINAL_ABILITY_W_EPC              (0x04)

enum{
    MODE_INITIATIVE     = 0,
    MODE_PASSIVITY,
};

#if 0

struct _app_runinfo_{
    CDeive_Base        *m_pdevice_am2305;
    CDeive_Base        *m_pdevice_e2;
    CDeive_Base        *m_pdevice_rfid[RFID_READER_MAX_NUMB];
    struct reader_info  m_readerinfo[RFID_READER_MAX_NUMB];
    struct epc_info     m_epcinfo;
    struct app_rfidinfo m_rfidinfo;
    uint8               m_ability;
    uint8               m_mode;
    uint8               m_status;
    uint8               m_reader_numbs;
    uint8               m_device_rfid_exist[RFID_READER_MAX_NUMB];
};
typedef struct _app_runinfo_ app_runinfo_t;

class CApplication{
public:
    static CApplication *GetInstance(void);
    portBASE_TYPE run(void);
    portBASE_TYPE init(void);

private:
    CApplication();
    ~CApplication();
    CApplication(const CApplication &other);
    CApplication &operator =(const CApplication &other);
    portBASE_TYPE protocol_rfid_read(void);
    portBASE_TYPE protocol_am2305(void);
    void content_readerinfo_make(uint8 *pbuf, uint16 *plen);
    uint8 protocol_rfid_write(uint8 *pbuf, uint16 len);
    portBASE_TYPE readerrfid_init(void);
    portBASE_TYPE readerrfid_write(uint8 *pbuf, uint16 *plen);
    portBASE_TYPE containerrfid_r_epc(CDevice_Rfid  *pdevice_rfid);
    portBASE_TYPE containerrfid_w_epc(CDevice_Rfid  *pdevice_rfid, uint8 *pepc);
    portBASE_TYPE containerrfid_r_data(CDevice_Rfid *pdevice_rfid, uint8 index, uint8 *pbuff, uint16 *plen, uint8 ctrl);
    portBASE_TYPE containerrfid_w_data(CDevice_Rfid *pdevice_rfid, uint8 epc_len, uint8 *pepc, uint8 *pdata);
    portBASE_TYPE device_status_send(void);
    CDeive_Base *rfid_device_get(uint8 index);
    static portBASE_TYPE package_event_handler(uint8 func_code, uint8 *pbuf, uint16 len);


    static CApplication     *m_pcapplicaiton;
    struct _app_runinfo_    m_app_runinfo;
};

#endif










#endif


