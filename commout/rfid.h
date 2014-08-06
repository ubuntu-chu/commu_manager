#ifndef _RFID_H_
#define _RFID_H_

#include <includes/includes.h>

#define         DEV_ONLINE                              (1)
#define         DEV_OFFLINE                             (0)

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

	uint8           m_id_;              //阅读器id
	uint8           m_offline_cnt_;     //离线次数
	uint8           m_exist_;           //在线信息
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

//unit: s
#define         def_RFID_DEF_MAX_WAIT_TIME          (2)

class channel;

class CDevice_Rfid:boost::noncopyable{
public:
    CDevice_Rfid():max_wait_time_(def_RFID_DEF_MAX_WAIT_TIME),
        reader_id_(0),
        offset_(4),
        status_(3){};
    ~CDevice_Rfid(){};

    void reader_id_set(uint8 id){reader_id_ = id;}
    void reader_no_set(uint8 no)
    {
        vector<struct reader_info>::iterator it;
        struct reader_info t_reader_info;
        int i;

        t_reader_info.m_offline_cnt_               = 0;
        t_reader_info.m_exist_                     = DEV_OFFLINE;
        t_reader_info.m_power                      = 0;
        t_reader_info.m_scntm                      = def_RFID_DEF_MAX_WAIT_TIME*10;
        readerinfo_vec_.reserve(no);

        for(i = 0; i < no; i++){
            readerinfo_vec_.push_back(t_reader_info);
        }
    }
    int reader_status_get(uint8 id)
    {
        return readerinfo_vec_[id].m_exist_;
    }

    vector<struct reader_info> *reader_info_get(void)
    {
//        return &readerinfo_vec_[id];
        return &readerinfo_vec_;
    }

	portBASE_TYPE query_rfid(struct epc_info *pinfo);
	portBASE_TYPE query_readerinfo(struct reader_info *info);
	portBASE_TYPE querytime_set(uint8 scantime);
	portBASE_TYPE power_set(uint8 power);
	portBASE_TYPE sound_set(uint8 on);
	portBASE_TYPE read_data(struct read_info *pinfo, uint8 *pbuf);
	portBASE_TYPE write_data(struct write_info *pinfo, uint8 *pbuf);
	static portBASE_TYPE epc_get(struct epc_info *pinfo, uint8 numb, uint8 *penumb, uint8 *pepc);
	int channel_power_get(void);

	portBASE_TYPE max_wait_time_restore(void)
	{
        readerinfo_vec_[reader_id_].m_scntm = def_RFID_DEF_MAX_WAIT_TIME*10;
        return 0;
    }

	int rfid_device_online_no_get(void)
	{
        vector<struct reader_info>::iterator it;
        int no  = 0;

        //找到第index个在线设备
        for(it = readerinfo_vec_.begin(); it != readerinfo_vec_.end(); ++it){
            if (it->m_exist_ == DEV_ONLINE){
                no++;
            }
        }

        return no;
	}

	int rfid_device_id_get(int index)
    {
        vector<struct reader_info>::iterator it;
        int i  = 0;

        //找到第index个在线设备
        for(it = readerinfo_vec_.begin(); it != readerinfo_vec_.end(); ++it){
            if (it->m_exist_ == DEV_ONLINE){
                if (i == index){
                    break;
                }
                i++;
            }
        }

        return it->m_id_;
    }

	list_head_t *device_list_head_get();
    int device_no_get(void);

	void channel_set(channel *pchannel){pchannel_    = pchannel;}
	channel *channel_get(void){return pchannel_;}

private:
    CDevice_Rfid(const CDevice_Rfid &other);
    CDevice_Rfid &operator =(const CDevice_Rfid &other);

    int channel_write_sync_inloop(vector<char> &vec, int wait_time, vector<char> **ppvec_ret);
    void log_critical_print(const char *func, const char *msg);
	void log_print(const char *func, int rt, vector<char> *pvec_ret);


	struct rfid_rsp_info		m_rsp_info;
	int                         max_wait_time_;
	int                         recv_data_len_;
	uint8                       reader_id_;
	uint8                       offset_;
	uint8                       status_;
	channel                     *pchannel_;

    vector<struct reader_info>  readerinfo_vec_;
};    
    
    













#endif


