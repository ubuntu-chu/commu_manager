#include "rfid.h"
#include "stdio.h"

//command list
enum rfid_g2_cmd {
	QUERY_RFID				= 0x01,
	READ_DATA				= 0x02,
	WRITE_DATA				= 0x03,
	WRITE_EPC				= 0x04,
	DESTROY_RFID			=0x05,
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
	READER_BPS				=0x28,
	READER_POWER			= 0x2f,
	READER_SNDLIGHT			= 0x33,
	READER_WORKMODE			= 0x35,
	READER_WORKMODEPARAM    = 0x36,
};

//static const uint8	rfid_def_addr	= 0xff;
static const uint8	rfid_def_addr	= 0;

#if 0

#define 	BUF_SEND_CMD()		(m_buffsend[2])
#define 	BUF_SEND_CMD_SET(cmd)	(m_buffsend[2] = cmd)
#define 	BUF_SEND_ADDR()	(m_buffsend[1])
#define 	BUF_SEND_ADDR_DEF_SET()	(m_buffsend[1] = rfid_def_addr)
#define 	BUF_SEND_LEN_SET()	(m_buffsend[0] = m_len_send + 1)
#define		BUF_LEN_INC()		(m_len_send++)
//min = 4
#define		BUF_LEN_INIT()		(m_len_send = 3)
#define 	BUF_SEND_DATA_PUSH(data)	do{ m_buffsend[m_len_send] = data; m_len_send++;}while(0)
//#define 	BUF_SEND_DATA_PUSH(data)	do{ m_buffsend[m_len_send] = data; m_len_send++; m_buffsend[0]++}while(0)


static unsigned int uiCrc16Cal(unsigned char const  * pucY, unsigned char ucX);
static void send_buf_dump(void);

CDevice_Rfid::CDevice_Rfid(const char *pname, uint16 oflag):CDeive_Base(pname, oflag)
{
//    memset(m_buffsend, 0, sizeof(m_buffsend));
//    memset(m_buffrecv, 0, sizeof(m_buffrecv));
	m_len_send			 = 0;
	m_len_recv	     	 = 0;
	m_pbuf_recv			= m_buffrecv;
	m_pbuf_send			= m_buffsend;
	//buf recv max len
    m_buf_recv_len      = sizeof(m_buffrecv);

}

CDevice_Rfid::~CDevice_Rfid()
{

}

portBASE_TYPE CDevice_Rfid::max_wait_time_set(uint8 wait_time)
{
    API_DeviceControl(m_pdevice, DEV_RFID_MAX_WAIT_TIME_SET, (void *)wait_time);

	return 0;
}

portBASE_TYPE CDevice_Rfid::process_read(void)
{
	//check frame valid  
	uint16		crc;

	//caculate crc
	crc		= uiCrc16Cal(m_buffrecv, m_len_recv);	
	if (0 != crc){
		return -1;
	}
	m_rsp_info.m_recmd		= m_buffrecv[2];
	m_rsp_info.m_addr		= m_buffrecv[1];
	m_rsp_info.m_status		= m_buffrecv[3];
	m_len_data				= m_buffrecv[0] - 5;
	//get rsp data seg
	if (m_len_data != 0){
		m_pbuf_data			= &m_buffrecv[4];
	}else {
		m_pbuf_data			= NULL;
	}
	//if ((m_rsp_info.m_status == 0) && (BUF_SEND_CMD() == m_rsp_info.m_recmd) && (BUF_SEND_ADDR() == m_rsp_info.m_addr)){
	if ((BUF_SEND_CMD() == m_rsp_info.m_recmd) && (BUF_SEND_ADDR() == m_rsp_info.m_addr)){
		m_rsp_info.m_valid	= TRUE;
//        SYS_LOG_TINY("frame recv valide-----\n");
	}else {
		m_rsp_info.m_valid	= FALSE;
//        SYS_LOG_TINY("frame recv invalide!!!!-----\n");
	}
#if 0
	//------------------debug-------------------
	SYS_LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	SYS_LOG("recv frame is %s\n", (m_rsp_info.m_valid== TRUE)?("valid"):("invalid"));
	SYS_LOG("recv len = %d\n", m_buffrecv[0]);
	SYS_LOG("recv addr = 0x%x\n", m_rsp_info.m_addr);
	SYS_LOG("recv recmd = 0x%x\n", m_rsp_info.m_recmd);
	SYS_LOG("recv status = 0x%x\n", m_rsp_info.m_status);
	if (m_len_data){
		uint16		i;
        SYS_LOG("recv data ------------->\n");
		SYS_LOG("recv data len = %d\n", m_len_data);
		for (i = 0; i < m_len_data; i++){
			SYS_LOG("recv data[%d] = 0x%x\n", i, (uint8)m_pbuf_data[i]);
		}
	}
	SYS_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
#endif
    
    return (m_rsp_info.m_valid == TRUE)?(0):(-1);
}


portBASE_TYPE CDevice_Rfid::process_write(char *pbuf, uint16 size)
{
	uint16	crc;
	uint16  i;

	//construct protocol cmd
	BUF_LEN_INIT();
	BUF_SEND_CMD_SET(pbuf[0]);
	BUF_SEND_ADDR_DEF_SET();
	for (i = 1; i < size; i++){
		//push data
		BUF_SEND_DATA_PUSH(pbuf[i]);
	}
	//set buf[0] len
	BUF_SEND_LEN_SET();
	//caculate crc
	crc		= uiCrc16Cal(m_buffsend, m_len_send);	
	//push crc
	BUF_SEND_DATA_PUSH(crc & 0x00ff);
	BUF_SEND_DATA_PUSH(crc >> 8);

	//send_buf_dump();

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
	uint8	buf_send[1];
	uint8	buf_recv[300];
	portSIZE_TYPE	buf_recv_len;

	buf_send[0]			= QUERY_RFID;
	write((char *)buf_send, sizeof(buf_send));
	buf_recv_len = read((char *)buf_recv, sizeof(buf_recv));
	if ((buf_recv_len == (portSIZE_TYPE)-1)
		|| (m_rsp_info.m_status != 1)){
		sprintf((char *)buf_recv, "Err: query rfid: status = 0x%x\n", m_rsp_info.m_status);
        SYS_LOG_LEV_TINY(SYS_LOG_LEV_DEBUG, buf_recv);    
        //SYS_LOG("Err: query rfid: status = 0x%x\n", m_rsp_info.m_status);
		return -1;
	}
	if (NULL != pinfo){
		memcpy((void *)pinfo, (void *)buf_recv, buf_recv_len);
		pinfo->m_len		= buf_recv_len;
	}
	
	return 0;
}

portBASE_TYPE CDevice_Rfid::read_data(struct read_info *pinfo, uint8 *pbuf)
{
	uint8	buf_send[100];
	uint8	buf_recv[100];
	portSIZE_TYPE	buf_recv_len;

	//prepare read info  
	//pwd fix with 0   epc must be 12 bytes
	pinfo->m_pwd[0]		= 0x00;
	pinfo->m_pwd[1]		= 0x00;
	pinfo->m_pwd[2]		= 0x00;
	pinfo->m_pwd[3]		= 0x00;
	//fill buf send
	buf_send[0]			= READ_DATA;
	memcpy((void *)&buf_send[1], (void *)pinfo, sizeof(struct read_info));
	write((char *)buf_send, sizeof(read_info) + 1);
	buf_recv_len = read((char *)buf_recv, sizeof(buf_recv));
	if ((m_rsp_info.m_status != 0) 
		|| (buf_recv_len == (portSIZE_TYPE)-1)){
		SYS_LOG("Err: read data: status = 0x%x\n", m_rsp_info.m_status);
		return -1;
	}
	if (NULL != pbuf){
		memcpy((void *)pbuf, (void *)buf_recv, buf_recv_len);
	}

	return 0;
}

portBASE_TYPE CDevice_Rfid::write_data(struct write_info *pinfo, uint8 *pbuf)
{
	uint8	buf_send[100];
	uint8	buf_recv[100];
	uint8   buf_send_len;
	portSIZE_TYPE	buf_recv_len;

	//prepare read info  
	//pwd fix with 0   epc must be 12 bytes
	pinfo->m_pwd[0]		= 0x00;
	pinfo->m_pwd[1]		= 0x00;
	pinfo->m_pwd[2]		= 0x00;
	pinfo->m_pwd[3]		= 0x00;
	//fill buf send
	buf_send[0]			= WRITE_DATA;
	//sizeof(struct write_info) - 4  sub psw len
	memcpy((void *)&buf_send[1], (void *)pinfo, sizeof(struct write_info) - 4);
	buf_send_len		= sizeof(struct write_info) - 3;
	//copy wirte data
	memcpy((void *)&buf_send[buf_send_len], (void *)pbuf, (pinfo->m_wnum)<<1);
	buf_send_len		+= (pinfo->m_wnum)<<1;
	//copy pwd
	memcpy((void *)&buf_send[buf_send_len], (void *)pinfo->m_pwd, 4);
	buf_send_len		+= 4;
	write((char *)buf_send, buf_send_len);
	buf_recv_len = read((char *)buf_recv, sizeof(buf_recv));
	if ((m_rsp_info.m_status != 0) 
		|| (buf_recv_len == (portSIZE_TYPE)-1)){
		SYS_LOG("Err: write data: status = 0x%x\n", m_rsp_info.m_status);
		return -1;
	}

	return 0;
}

portBASE_TYPE CDevice_Rfid::query_readerinfo(struct reader_info *info)
{
	uint8	buf_send[1];
	uint8	buf_recv[100];
	portSIZE_TYPE	buf_recv_len;

	buf_send[0]			= READER_INFO;
	write((char *)buf_send, sizeof(buf_send));
	buf_recv_len = read((char *)buf_recv, sizeof(buf_recv));
	if ((m_rsp_info.m_status != 0) 
		|| (buf_recv_len == (portSIZE_TYPE)-1)){
		SYS_LOG("Err: query reader info: status = 0x%x\n", m_rsp_info.m_status);
		return -1;
	}
	if (NULL != info){
		info->m_version_hi		= buf_recv[0];
		info->m_version_low		= buf_recv[1];
		info->m_type			= buf_recv[2];
		info->m_tr_type			= buf_recv[3];
		info->m_dmaxfre			= buf_recv[4] & 0x3f;
		info->m_dminfre			= buf_recv[5] & 0x3f;
		info->m_power			= buf_recv[6];
		info->m_scntm			= buf_recv[7];
		info->m_freqband		= (enum freqband)(buf_recv[4]>>6 + buf_recv[5]>>6);
	}

	return 0;
}

portBASE_TYPE CDevice_Rfid::querytime_set(uint8 scantime)
{
	uint8	buf_send[2];
	uint8	buf_recv[100];
	portSIZE_TYPE	buf_recv_len;

	buf_send[0]			= READER_TIME;
	if (scantime < 3){
		scantime		= 3;
	}
	buf_send[1]			= scantime;
	write((char *)buf_send, sizeof(buf_send));
	buf_recv_len = read((char *)buf_recv, sizeof(buf_recv));
	if ((m_rsp_info.m_status != 0) 
		|| (buf_recv_len == (portSIZE_TYPE)-1)){
		SYS_LOG("Err: query time set: status = 0x%x\n", m_rsp_info.m_status);
		return -1;
	}

	return 0;
}


portBASE_TYPE CDevice_Rfid::power_set(uint8 power)
{
	uint8	buf_send[2];
	uint8	buf_recv[100];
	portSIZE_TYPE	buf_recv_len;

	buf_send[0]			= READER_POWER;
	if (power > 30){
		power			= 30;
	}
	buf_send[1]			= power;
	write((char *)buf_send, sizeof(buf_send));
	buf_recv_len = read((char *)buf_recv, sizeof(buf_recv));
	if ((m_rsp_info.m_status != 0) 
		|| (buf_recv_len == (portSIZE_TYPE)-1)){
		SYS_LOG("Err: power set: status = 0x%x\n", m_rsp_info.m_status);
		return -1;
	}

	return 0;
}

void CDevice_Rfid::send_buf_dump(void)
{
	SYS_LOG("send buff len = %d\n", m_len_send);

	buf_dump((uint8 *)m_buffsend, m_len_send);
}

#endif

#define PRESET_VALUE (0xFFFF)
#define POLYNOMIAL  (0x8408)

static unsigned int uiCrc16Cal(unsigned char const  * pucY, unsigned char ucX) 
{ 
    unsigned char ucI,ucJ; 
    unsigned short int  uiCrcValue = PRESET_VALUE; 

//	SYS_LOG("ucX = %d\n", ucX);
 
    for (ucI = 0; ucI < ucX; ucI++){
        
        uiCrcValue = uiCrcValue ^ *(pucY + ucI); 
        for (ucJ = 0; ucJ < 8; ucJ++){ 
	        if(uiCrcValue & 0x0001) { 
                uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL; 
	        } else{ 
	            uiCrcValue = (uiCrcValue >> 1); 
	        } 
    	} 
    }
   
    return uiCrcValue; 
}


