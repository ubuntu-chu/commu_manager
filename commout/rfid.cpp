#include "rfid.h"
#include "stdio.h"
#include "channel.h"
#include <datum.h>

extern struct process_stat   *process_stat_ptr_get(void);

//command list
enum rfid_g2_cmd {
	QUERY_RFID				= 0x01,
	READ_DATA				= 0x02,
	WRITE_DATA				= 0x03,
	WRITE_EPC				= 0x04,
	DESTROY_RFID			= 0x05,
	CMD_06					= 0x06,
	CMD_07					= 0x07,
	CMD_08					= 0x08,
	CMD_09					= 0x09,
	CMD_0a					= 0x0a,
	CMD_0b					= 0x0b,
	CMD_0c					= 0x0c,
	CMD_0d					= 0x0d,
	CMD_0e					= 0x0e,
	QUERY_SING				= 0x0f,
	BLCOK_WRITE				= 0x10,
};
enum rfid_reader_cmd{
	READER_INFO				= 0x21,
	READER_FREQ				= 0x22,
	READER_ADDR				= 0x24,
	READER_TIME				= 0x25,
	READER_BPS				= 0x28,
	READER_POWER			= 0x2f,
	READER_SNDLIGHT			= 0x33,
	READER_WORKMODE			= 0x35,
	READER_WORKMODEPARAM    = 0x36,
};


portBASE_TYPE CDevice_Rfid::epc_get(struct epc_info *pinfo, uint8 numb, uint8 *penumb, uint8 *pepc)
{
	uint8		*pbuf;
	uint8		enumb;
	uint8		eindex			= 0;

	if (numb > pinfo->m_numb){
		return -1;
	}
	pbuf		= pinfo->m_epcarray;
	while (eindex <= numb){
		enumb					= pbuf[0];
		if (eindex == numb){
			break;
		}
		pbuf					= pbuf + enumb + 1;
		eindex++;
	};
	if (NULL != penumb){
		*penumb					= enumb >> 1;
	}
	if (NULL != pepc){
		memcpy((void *)pepc, (void *)(pbuf + 1), enumb);
	}

	return 0;
}

extern void run_led_on(void);
extern void run_led_off(void);

int CDevice_Rfid::channel_write_sync_inloop(vector<char> &vec, int wait_time, vector<char> **ppvec_ret)
{
    int            rt;
    int            max_wait_time;
    struct process_stat   *pprocess_stat;

    if (pchannel_ == NULL){
        return -1;
    }
    //调用通道写函数
    //readerinfo_vec_[reader_id_].m_scntm 单位100ms
    max_wait_time                   = readerinfo_vec_[reader_id_].m_scntm/10;
    if (0 == max_wait_time){
        max_wait_time               = 1;
    }
    rt = pchannel_->write_sync_inloop(vec, max_wait_time, ppvec_ret);

    //rt > 0 代表操作超时
    if (rt > 0){
        readerinfo_vec_[reader_id_].m_offline_cnt_++;
        //与设备通讯超时3次后 认为设备离线
        if (readerinfo_vec_[reader_id_].m_offline_cnt_ > 1){
            readerinfo_vec_[reader_id_].m_exist_    = DEV_OFFLINE;
        }
    }else {
        readerinfo_vec_[reader_id_].m_offline_cnt_  = 0;
        readerinfo_vec_[reader_id_].m_exist_        = DEV_ONLINE;
    }
    pprocess_stat                                   = process_stat_ptr_get();
    //没有在线设备  通讯异常
    if (0 == rfid_device_online_no_get()){
        pprocess_stat->comm_stat                    = def_PROCESS_COMM_FAILED;
    }else {
        pprocess_stat->comm_stat                    = def_PROCESS_COMM_OK;
    }

    return rt;
}

void CDevice_Rfid::log_print(const char *func, int rt, vector<char> *pvec_ret)
{
    LOG_WARN << "Err: " << func << " failed! info: rt = [" << rt << "]";
    //当不是由超时引发的错误时
    if (rt <= 0){
        LOG_WARN << "status = [" << muduo::Fmt("0x%x", static_cast<unsigned char>((*pvec_ret)[status_])) << "]";
    }
}


portBASE_TYPE CDevice_Rfid::query_rfid(struct epc_info *pinfo)
{
    vector<char>   vec_send;
    vector<char>   *pvec_ret;
    int            rt;

    //组装命令
    vec_send.push_back(reader_id_);
    vec_send.push_back(QUERY_RFID);

    //调用通道写函数
    rt = channel_write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if ((0 != rt) || ((*pvec_ret)[status_] != 1)){
        log_print(__func__, rt, pvec_ret);
        return -1;
    }

	if (NULL != pinfo){
		memcpy((void *)pinfo, (void *)(&(*pvec_ret)[offset_]),
		        (*pvec_ret).size());
		pinfo->m_len		= (*pvec_ret).size();
	}
	
	return 0;
}

portBASE_TYPE CDevice_Rfid::read_data(struct read_info *pinfo, uint8 *pbuf)
{
    vector<char>   vec_send;
    vector<char>   *pvec_ret;
    int            rt;
    unsigned int  i;
    char          *pdata    = reinterpret_cast<char *>(pinfo);

    //prepare read info
    //pwd fix with 0   epc must be 12 bytes
    pinfo->m_pwd[0]     = 0x00;
    pinfo->m_pwd[1]     = 0x00;
    pinfo->m_pwd[2]     = 0x00;
    pinfo->m_pwd[3]     = 0x00;
    //组装命令
    vec_send.push_back(reader_id_);
    vec_send.push_back(READ_DATA);

    for (i = 0; i < sizeof(struct read_info); i++){
        vec_send.push_back(*pdata++);
    }

    //调用通道写函数
    rt = channel_write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if ((0 != rt) || ((*pvec_ret)[status_] != 0)){
        log_print(__func__, rt, pvec_ret);
        return -1;
    }

	if (NULL != pbuf){
		//-6即减去len addr recmd status lsb-crc16 msb-crc16
		memcpy((void *)pbuf, (void *)(&(*pvec_ret)[offset_]),
		        (*pvec_ret).size()-6);
	}

	return 0;
}

portBASE_TYPE CDevice_Rfid::write_data(struct write_info *pinfo, uint8 *pbuf)
{
    vector<char>   vec_send;
    vector<char>   *pvec_ret;
    int            rt;
    unsigned int  i;
    char          *pdata    = reinterpret_cast<char *>(pinfo);

    //prepare read info
    //pwd fix with 0   epc must be 12 bytes
    pinfo->m_pwd[0]     = 0x00;
    pinfo->m_pwd[1]     = 0x00;
    pinfo->m_pwd[2]     = 0x00;
    pinfo->m_pwd[3]     = 0x00;
    //组装命令
    vec_send.push_back(reader_id_);
    vec_send.push_back(WRITE_DATA);

    //sizeof(struct write_info) - 4  sub psw len
    for (i = 0; i < sizeof(struct read_info) - 4; i++){
        vec_send.push_back(*pdata++);
    }

    pdata                   = reinterpret_cast<char *>(pbuf);
    for (i = 0; i < static_cast<unsigned int>((pinfo->m_wnum)<<1); i++){
        vec_send.push_back(*pdata++);
    }

    pdata                   = reinterpret_cast<char *>(pinfo->m_pwd);
    for (i = 0; i < sizeof(pinfo->m_pwd); i++){
        vec_send.push_back(*pdata++);
    }
    //调用通道写函数
    rt = channel_write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if ((0 != rt) || ((*pvec_ret)[status_] != 0)){
        log_print(__func__, rt, pvec_ret);
        return -1;
    }

	return 0;
}

portBASE_TYPE CDevice_Rfid::query_readerinfo(struct reader_info *info)
{
    vector<char>   vec_send;
    vector<char>   *pvec_ret;
    int            rt;

    //组装命令
    vec_send.push_back(reader_id_);
    vec_send.push_back(READER_INFO);

    //调用通道写函数
    rt = channel_write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    readerinfo_vec_[reader_id_].m_id_             = reader_id_;
    readerinfo_vec_[reader_id_].m_power           = 0;
    if ((0 != rt) || ((*pvec_ret)[status_] != 0)){
        log_print(__func__, rt, pvec_ret);
        return -1;
    }
    //更新设备的扫描时间
    readerinfo_vec_[reader_id_].m_version_hi      = static_cast<uint8>((*pvec_ret)[offset_+0]);
    readerinfo_vec_[reader_id_].m_version_low     = static_cast<uint8>((*pvec_ret)[offset_+1]);
    readerinfo_vec_[reader_id_].m_type            = static_cast<uint8>((*pvec_ret)[offset_+2]);
    readerinfo_vec_[reader_id_].m_tr_type         = static_cast<uint8>((*pvec_ret)[offset_+3]);
    readerinfo_vec_[reader_id_].m_dmaxfre         = static_cast<uint8>((*pvec_ret)[offset_+4]) & 0x3f;
    readerinfo_vec_[reader_id_].m_dminfre         = static_cast<uint8>((*pvec_ret)[offset_+5]) & 0x3f;
    readerinfo_vec_[reader_id_].m_power           = static_cast<uint8>((*pvec_ret)[offset_+6]);
    readerinfo_vec_[reader_id_].m_scntm           = static_cast<uint8>((*pvec_ret)[offset_+7]);
    readerinfo_vec_[reader_id_].m_freqband        = (enum freqband)((static_cast<uint8>((*pvec_ret)[offset_+4])>>6)
                                                        + (static_cast<uint8>((*pvec_ret)[offset_+5])>>6));
    if (NULL != info){
        *info                  = readerinfo_vec_[reader_id_];
#if 0
        info->m_version_hi      = static_cast<uint8>((*pvec_ret)[offset_+0]);
        info->m_version_low     = static_cast<uint8>((*pvec_ret)[offset_+1]);
        info->m_type            = static_cast<uint8>((*pvec_ret)[offset_+2]);
        info->m_tr_type         = static_cast<uint8>((*pvec_ret)[offset_+3]);
        info->m_dmaxfre         = static_cast<uint8>((*pvec_ret)[offset_+4]) & 0x3f;
        info->m_dminfre         = static_cast<uint8>((*pvec_ret)[offset_+5]) & 0x3f;
        info->m_power           = static_cast<uint8>((*pvec_ret)[offset_+6]);
        info->m_scntm           = static_cast<uint8>((*pvec_ret)[offset_+7]);
        info->m_freqband        = (enum freqband)((static_cast<uint8>((*pvec_ret)[offset_+4])>>6)
                                    + (static_cast<uint8>((*pvec_ret)[offset_+5])>>6));
#endif
    }

	return 0;
}

portBASE_TYPE CDevice_Rfid::querytime_set(uint8 scantime)
{
    vector<char>   vec_send;
    vector<char>   *pvec_ret;
    int            rt;

    //组装命令
    vec_send.push_back(reader_id_);
    vec_send.push_back(READER_TIME);
    if (scantime < RFID_READER_MIN_SCNTIME){
        scantime        = RFID_READER_MIN_SCNTIME;
    }
    vec_send.push_back(scantime);

    //调用通道写函数
    rt = channel_write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if ((0 != rt) || ((*pvec_ret)[status_] != 0)){
        log_print(__func__, rt, pvec_ret);
        return -1;
    }

	return 0;
}


portBASE_TYPE CDevice_Rfid::power_set(uint8 power)
{
    vector<char>   vec_send;
    vector<char>   *pvec_ret;
    int            rt;

    //组装命令
    vec_send.push_back(reader_id_);
    vec_send.push_back(READER_POWER);
    if (power > RFID_READER_MAX_POWER){
        power        = RFID_READER_MAX_POWER;
    }
    vec_send.push_back(power);

    //调用通道写函数
    rt = channel_write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if ((0 != rt) || ((*pvec_ret)[status_] != 0)){
        log_print(__func__, rt, pvec_ret);
        return -1;
    }

	return 0;
}

portBASE_TYPE CDevice_Rfid::sound_set(uint8 on)
{
    vector<char>   vec_send;
    vector<char>   *pvec_ret;
    int            rt;
    char           mode_state;
    char           read_mode_offset = 8;
    char           read_mode;

    //组装命令
    vec_send.push_back(reader_id_);
    vec_send.push_back(READER_WORKMODEPARAM);

    //调用通道写函数
    rt = channel_write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if ((0 != rt) || ((*pvec_ret)[status_] != 0)){
        log_print(__func__, rt, pvec_ret);
        return -1;
    }

    vec_send.clear(); 
    vec_send.push_back(reader_id_);
    vec_send.push_back(READER_WORKMODE);
    //先读取工作模式参数  然后再进行设置
    //read_mode:工作模式选择 应答模式
    read_mode               =  (*pvec_ret)[read_mode_offset++];
    vec_send.push_back(2);
    //mode_state
    //bit0  bit0=0 读写器支持18000-6c协议
    //bit1  bit1=1 rs232/rs485输出
    //bit2  bit2=0 buzzer on   bit2=1 buzzer 0ff
    mode_state              = (*pvec_ret)[read_mode_offset++];
    if (on){
        mode_state          &= ~0x04;
    }else {
        mode_state          |= 0x04;
    }
    vec_send.push_back(mode_state);
    //mem_inven
    vec_send.push_back((*pvec_ret)[read_mode_offset++]);
    //first_adr
    vec_send.push_back((*pvec_ret)[read_mode_offset++]);
    //word_num
    vec_send.push_back((*pvec_ret)[read_mode_offset++]);
    //tag_time
    vec_send.push_back((*pvec_ret)[read_mode_offset++]);

    //调用通道写函数
    rt = channel_write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if ((0 != rt) || ((*pvec_ret)[status_] != 0)){
        log_print(__func__, rt, pvec_ret);
        return -1;
    }

    vec_send[2]             = read_mode;
    //调用通道写函数
    rt = channel_write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if ((0 != rt) || ((*pvec_ret)[status_] != 0)){
        log_print(__func__, rt, pvec_ret);
        return -1;
    }
 
    return 0;
}

list_head_t *CDevice_Rfid::device_list_head_get()
{
    return pchannel_->device_list_head_get();
}

int CDevice_Rfid::device_no_get(void)
{
    return pchannel_->device_no_get();
}

