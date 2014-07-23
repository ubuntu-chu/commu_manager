#ifndef _RFID_H_
#define _RFID_H_

#include <includes/includes.h>

struct rfid_protocol_cmd{
	uint8			m_len;
	uint8			m_addr;
	uint8			m_cmd;
	uint8			m_data[100];
	uint8			m_crclow;
	uint8			m_crchigh;
};

struct rfid_protocol_rsp{
	uint8			m_len;
	uint8			m_addr;
	uint8			m_recmd;
	uint8			m_status;
	uint8			m_data[256];
	uint8			m_crclow;
	uint8			m_crchigh;
};

struct rfid_rsp_info{
	uint8			m_valid;
	uint8			m_status;
	uint8			m_recmd;
	uint8           m_addr;
};


struct epc_info{
	uint8			m_numb;
	//max epc array len is 255
	uint8		    m_epcarray[255];
	uint8			m_len;
};

enum freqband{
	USER_BAND	= 0,
	CHINESE_BAND,
	US_BAND,
	KOREAN_BAND,
};

struct reader_info{
	uint8			m_version_hi;
	uint8			m_version_low;
	uint8			m_type;
	uint8			m_tr_type;
	uint8			m_dmaxfre;
	uint8			m_dminfre;
	uint8			m_power;
	//query time
	uint8			m_scntm;
	enum freqband   m_freqband;
};

enum memory{
	MEM_RESERVE		= 0,
	MEM_EPC,
	MEM_TID,
	MEM_USER,
};

struct read_info{
	uint8_t			m_enum;
	//epc own a fixed len
	uint8_t			m_epcarray[12];
	uint8_t 		m_mem;
	//word index
	uint8_t			m_wordptr;
	uint8_t			m_num;
	uint8_t			m_pwd[4];
};

struct write_info{
	//word len   1 word = 2 bytes
	uint8_t			m_wnum;
	uint8_t			m_enum;
	//epc own a fixed len
	uint8_t			m_epcarray[12];
	uint8_t 		m_mem;
	//word index
	uint8_t			m_wordptr;
	uint8_t			m_pwd[4];
};

#if 0

class CDevice_Rfid:public CDeive_Base{
public:
    CDevice_Rfid(const char *pname, uint16 oflag);
    virtual ~CDevice_Rfid();
	portBASE_TYPE query_rfid(struct epc_info *pinfo);
	portBASE_TYPE query_readerinfo(struct reader_info *info);
	portBASE_TYPE querytime_set(uint8 scantime);
	portBASE_TYPE power_set(uint8 power);
	portBASE_TYPE read_data(struct read_info *pinfo, uint8 *pbuf);
	portBASE_TYPE write_data(struct write_info *pinfo, uint8 *pbuf);
	portBASE_TYPE max_wait_time_set(uint8 wait_time);
	static portBASE_TYPE epc_get(struct epc_info *pinfo, uint8 numb, uint8 *penumb, uint8 *pepc);
private:
    CDevice_Rfid(const CDevice_Rfid &other);
    CDevice_Rfid &operator =(const CDevice_Rfid &other);
    virtual portBASE_TYPE process_read(void);
    virtual portBASE_TYPE process_write(char *pbuf, uint16 size);
	void send_buf_dump(void);
    uint8					m_buffsend[50];
    uint8					m_buffrecv[260];

	struct rfid_rsp_info		m_rsp_info;
};    
    
#endif
    













#endif


