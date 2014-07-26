#include "rfid.h"
#include "stdio.h"
#include "channel.h"

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

portBASE_TYPE CDevice_Rfid::max_wait_time_set(uint8 wait_time)
{
    max_wait_time_                      = wait_time;

	return 0;
}

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

portBASE_TYPE CDevice_Rfid::query_rfid(struct epc_info *pinfo)
{
    vector<char>   vec_send;
    vector<char>   *pvec_ret;
    int            rt;

    //组装命令
    vec_send.push_back(reader_id_);
    vec_send.push_back(QUERY_RFID);

    //调用通道写函数
    rt = pchannel_->write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if (0 != rt){
        LOG_ERROR << "Err: " << __func__ << "failed! info: status = [" << rt << "]";
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
    rt = pchannel_->write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if (0 != rt){
        LOG_ERROR << "Err: " << __func__ << "failed! info: status = [" << rt << "]";
        return -1;
    }

	if (NULL != pbuf){
		memcpy((void *)pbuf, (void *)(&(*pvec_ret)[offset_]),
		        (*pvec_ret).size());
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
    rt = pchannel_->write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if (0 != rt){
        LOG_ERROR << "Err: " << __func__ << "failed! info: status = [" << rt << "]";
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
    rt = pchannel_->write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if (0 != rt){
        LOG_ERROR << "Err: " << __func__ << "failed! info: status = [" << rt << "]";
        return -1;
    }
    if (NULL != info){
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
    if (scantime < 3){
        scantime        = 3;
    }
    vec_send.push_back(scantime);

    //调用通道写函数
    rt = pchannel_->write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if (0 != rt){
        LOG_ERROR << "Err: " << __func__ << "failed! info: status = [" << rt << "]";
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
    if (power > 30){
        power        = 30;
    }
    vec_send.push_back(power);

    //调用通道写函数
    rt = pchannel_->write_sync_inloop(vec_send, max_wait_time_, &pvec_ret);

    if (0 != rt){
        LOG_ERROR << "Err: " << __func__ << "failed! info: status = [" << rt << "]";
        return -1;
    }

	return 0;
}

list_head_t *CDevice_Rfid::device_list_head_get()
{
    return pchannel_->device_list_head_get();
}

