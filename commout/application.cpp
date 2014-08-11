#include "channel/channel.h"
#include "./io_node/ext_client.h"
#include "./io_node/inner_client.h"
#include "./io_node/inner_server.h"
#include "net.h"
#include <protocol_rfid.h>
#include "application.h"
#include <utils.h>
#include <datum.h>
#include <parse.h>

#define     def_DBG_IN_PC_SINGLE        (0)

#if (def_DBG_IN_PC_SINGLE > 0)
#define     PROCESS_PREFIX_PATH         "/home/barnard/work/commu_manager/manager/"
#endif

using std::string;

boost::scoped_ptr<muduo::LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
    g_logFile->append(msg, len);
    //因为在使用文件作为日志时  写入的数据存放在fp所自带的缓冲区中 并没有真正的写入文件 所以执行
    //flush强制进行写入
    g_logFile->flush();
}

void flushFunc()
{
    g_logFile->flush();
}

using namespace muduo;
using namespace muduo::net;

void signal_handle(int sign_no)
{
    if (sign_no == SIGINT){
//        t_project_datum.pevent_loop_->quit();
    }
}

struct process_stat   *g_pprocess_stat = NULL;

struct process_stat   *process_stat_ptr_get(void)
{
    return g_pprocess_stat;
}

void process_stat_set(int stat)
{
    g_pprocess_stat->comm_stat              = stat;
}

void process_stat_ptr_set(int index)
{
    //获取进程状态结构体数组指针
    t_project_datum.pprocess_stat_          = reinterpret_cast<struct process_stat   *>(t_project_datum.shmem_.attach());

    g_pprocess_stat   = t_project_datum.pprocess_stat_ + index;

    LOG_INFO << "process index = " << index << " g_pprocess_stat = " << reinterpret_cast<int *>(g_pprocess_stat);
}
//---------------------------------------------------------------

CApplication    *CApplication::m_pcapplicaiton = NULL;
class project_datum  t_project_datum;

static CApplication     c_application;

CApplication *CApplication::GetInstance(void)
{
    if (NULL == m_pcapplicaiton){
        m_pcapplicaiton         = &c_application;
    }

    return (m_pcapplicaiton);
}

static  CDevice_Rfid       s_Device_rfid;
static  CDevice_net        s_Device_net;

portBASE_TYPE CApplication::init(const char *log_file_path, const char *config_file_path)
{
	std::string      process_name_str;
	//依据配置资源 创建channel
//    boost::ptr_vector<channel> channel_vector;
//    vector<boost::shared_ptr<channel> > channel_vector;
	io_node         *pio_node;
	int             i, j;
	int             io_vector_no;
	int             channel_no              = 0;

	m_app_runinfo.m_status                  = enum_APP_STATUS_INIT;
	m_app_runinfo.m_pdevice_rfid            = &s_Device_rfid;
	m_app_runinfo.m_pdevice_net             = &s_Device_net;
	m_app_runinfo.m_ability                 = TERMINAL_ABILITY_NONE;
    m_app_runinfo.m_mode                    = MODE_INITIATIVE;
    m_app_runinfo.ppid_                     = ::getppid();

    //设置网络包处理函数
    m_app_runinfo.m_pdevice_net->package_event_handler_set(
            boost::bind(&CApplication::package_event_handler,
            this, _1, _2, _3));

//    utils::signal_handler_install(SIGINT, signal_handle);

    //获取进程名字
    process_name_str                        = ProcessInfo::procname();

    //设置日志文件名称
    g_logFile.reset(new muduo::LogFile(log_file_path, 10 * 1024 * 1024));
    muduo::Logger::setOutput(outputFunc);
    muduo::Logger::setFlush(flushFunc);

    //创建进程间共享内存
    if (t_project_datum.shmem_.create()){
        m_app_runinfo.m_status                  = enum_APP_STATUS_INIT_ERR;
		return -1;
    }
	LOG_INFO << "parse project xml config file: " << config_file_path;
	if (xml_parse(config_file_path, &t_project_datum.project_config_)){
        m_app_runinfo.m_status                  = enum_APP_STATUS_INIT_ERR;
		LOG_ERROR << "project xml config file parse failed!";

		return -1;
	}
//    t_project_datum.pproject_config_    = reinterpret_cast<project_config *>(t_project_datum.shmem_.attach());
	project_config	*pproject_config    = &t_project_datum.project_config_;
	io_config       &io_conf	        = pproject_config->io_config_get();
//    Logger::setLogLevel(static_cast<muduo::Logger::LogLevel>(pproject_config->log_lev_get()));
    Logger::setLogLevel(static_cast<muduo::Logger::LogLevel>(t_project_datum.project_config_.log_lev_get()));
//    Logger::setLogLevel(muduo::Logger::INFO);

    LOG_INFO  << "CApplication::init------------------------";

    //统计属于本进程的通道数量
    channel_no                          = 0;

    for (i = io_conf.io_type_start(); i < io_conf.io_type_end(); i++){
        io_vector_no                    = io_conf.io_vector_no_get(i);
        for (j = 0; j < io_vector_no; j++){
            pio_node                    = io_conf.io_vector_get(i, j);
            //查找io配置中属于当前进程的io_node
            if (0 == strcmp(process_name_str.c_str(), pio_node->process_get())){
                channel *pchannel = channel::channel_create(pio_node);
                m_app_runinfo.channel_vector_.push_back(pchannel);
//                channel_vector.push_back(channel::channel_create(pio_node));
                if (pchannel->contain_protocol(def_PROTOCOL_RFID_NAME)){

                    m_app_runinfo.m_pdevice_rfid->channel_set(pchannel);
                }
                if (pchannel->contain_protocol(def_PROTOCOL_MAC_NAME)){

                    m_app_runinfo.m_pdevice_net->channel_set(pchannel);
                }
                channel_no++;
                //打开通道电源
                pchannel->power_on();
            }
        }
    }

    //若进程内通道数目必须为2  则代表配置文件配置出错  此进程不做任何事情
    if (2 != channel_no){
        m_app_runinfo.m_status                  = enum_APP_STATUS_INIT_ERR;
		LOG_ERROR << "project xml config file error! the channels belongs to process numb != 2";

		return -2;
    }

	process_config  &process_conf	        = pproject_config->process_config_get();
	process_node    *pprocess_node;
	int             process_vector_no       = process_conf.process_vector_no_get();
	char            process_index           = process_vector_no;

    for (i = 0; i < process_vector_no; i++){
        pprocess_node                   = process_conf.process_node_get(i);

        if (0 == strcmp(process_name_str.c_str(), pprocess_node->name_get())){

            process_index                   = pprocess_node->index_get();
            //设置进程索引号
            process_stat_ptr_set(process_index);
            break;
        }
    }

    if (process_index >= process_vector_no){
        m_app_runinfo.m_status                  = enum_APP_STATUS_INIT_ERR;
		LOG_ERROR << "project xml config file error! process index >= process numb";

		return -3;
    }
    //初始化进程通讯状态  此状态是指 进程与阅读器之间状态
    process_stat_set(def_PROCESS_COMM_FAILED);

    //创建定时器  此定时器在多线程环境中使用  因此不能使用setitime 及 timer_create 此定时器仅用于查询1s是否到时
    m_app_runinfo.timer_fd_                 = ::timerfd_create(CLOCK_MONOTONIC,
                                                       TFD_NONBLOCK | TFD_CLOEXEC);
    if (m_app_runinfo.timer_fd_ < 0)
    {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
	//设定1s循环定时

    struct itimerspec new_value;

    //通过配置文件来配置心跳时间
    new_value.it_value.tv_sec               = t_project_datum.project_config_.heartbeat_s_get();
    new_value.it_value.tv_nsec              = 0;
    new_value.it_interval.tv_sec            = new_value.it_value.tv_sec;
    new_value.it_interval.tv_nsec           = new_value.it_value.tv_nsec;

    if (::timerfd_settime(m_app_runinfo.timer_fd_, 0, &new_value, NULL) == -1){
        LOG_SYSERR << "timerfd_settime()";
    }

    return 0;
}

void CApplication::content_readerinfo_make(uint8 *pbuf, uint16 *plen)
{
    uint16          len                     = 0;
    uint8           i                       = 0;
    CDevice_Rfid    *pdevice_rfid          = m_app_runinfo.m_pdevice_rfid;
    vector<struct reader_info> *preader_info = pdevice_rfid->reader_info_get();

    if (NULL == pbuf){
        return;
    }
    //capacity
    pbuf[len++]                         = m_app_runinfo.m_ability;
    //reader numbs
    pbuf[len++]                         = m_app_runinfo.m_reader_numbs;
    for (i = 0; i < m_app_runinfo.m_reader_numbs; ++i) {
        pbuf[len++]                     = (*preader_info)[i].m_id_;
        pbuf[len++]                     = (*preader_info)[i].m_exist_;
        pbuf[len++]                     = (*preader_info)[i].m_power;
        pbuf[len++]                     = (*preader_info)[i].m_scntm;
    }
    if (NULL != plen){
        *plen                           = len;
    }
}

portBASE_TYPE CApplication::readerrfid_query(uint8 *pbuf, uint16 *plen)
{
    CDevice_Rfid    *pdevice_rfid          = m_app_runinfo.m_pdevice_rfid;
    list_head_t     *pdevice_list_head;
    list_node_t     *pos;
    device_node     *pdevice_node;
    class device_rfid_reader_node   *pnode_rfid_reader;

    //获取通道下所挂接设备数量
    pdevice_list_head                       = pdevice_rfid->device_list_head_get();
    m_app_runinfo.m_reader_numbs            = pdevice_rfid->device_no_get();
    pdevice_rfid->reader_no_set(m_app_runinfo.m_reader_numbs);

    //遍历设备链表
    list_for_each(pos, pdevice_list_head){
        pdevice_node        = device_node::device_entry(pos);
        pnode_rfid_reader = reinterpret_cast<device_rfid_reader_node *>(pdevice_node);

        //查询设备信息
        pdevice_rfid->reader_id_set(pnode_rfid_reader->id_get());
        pdevice_rfid->max_wait_time_restore();
        pdevice_rfid->query_readerinfo(NULL);
    }
    content_readerinfo_make(pbuf, plen);

    return 0;
}

portBASE_TYPE CApplication::readerrfid_init(void)
{
    CDevice_net     *pdevice_net           = m_app_runinfo.m_pdevice_net;
    CDevice_Rfid    *pdevice_rfid          = m_app_runinfo.m_pdevice_rfid;
    static uint8   s_try_loop             = 3;
    uint8           buffer[500];
    portBASE_TYPE   rt;
    uint16          len                    = 0;
    int             rfid_device_online_no;

    readerrfid_query(buffer, &len);

    rfid_device_online_no               = pdevice_rfid->rfid_device_online_no_get();
    LOG_INFO << "rfid device config no [" << m_app_runinfo.m_reader_numbs
            << "]; exist no [" << rfid_device_online_no
            << "], now call pdevice_net->package_send_readerinfo with loop = ["
            << s_try_loop << "]";
    //when no reader exist, reader_numbs = 0
    //send reader info  to host
    rt     = pdevice_net->package_send_readerinfo((char *)buffer, len);

    utils::log_binary_buf("CApplication::readerrfid_init pdevice_net->package_send_readerinfo:",
            reinterpret_cast<const char *>(buffer), len);

    if (0 == rfid_device_online_no){
        LOG_WARN << "err:no rfid reader find! try scan again!";
        CurrentThread::sleepUsec(1*1000*1000);
        return -1;
    }
    if (rt != 0){
        LOG_WARN << "no respond from remote, sleep 1 and try again; loop = [" << s_try_loop << "]";
        CurrentThread::sleepUsec(1*1000*1000);

        return (s_try_loop-- == 0)?(0):(-2);
    }else {
    }
#if 0
    //1s time
    pdevice_rfid->querytime_set(500);
    pdevice_rfid->power_set(30);
    pdevice_rfid->query_readerinfo(&t_reader_info);
//    LOG_INFO << "reader power: type = %d, power = %d, query time = %d\n", m_app_runinfo.m_readerinfo.m_type, m_app_runinfo.m_readerinfo.m_power, m_app_runinfo.m_readerinfo.m_scntm);
#endif

    return 0;
}

portBASE_TYPE CApplication::readerrfid_sound_set(uint8 *pbuf, uint16 *plen)
{
    CDevice_Rfid    *pdevice_rfid          = m_app_runinfo.m_pdevice_rfid;
    uint8           rsp                     = RSP_OK;
    uint8           i;
    uint8           reader_id;
    uint8           *pdata;
    uint16          len                     = *plen;

    if (pbuf[0] != m_app_runinfo.m_reader_numbs){
        rsp                                 = RSP_INVALID_PARAM;
        goto quit;
    }
    //len check
    if (len != (1 + 2*(pbuf[0]))){
        rsp                                 = RSP_INVALID_PARAM_LEN;
        goto quit;
    }

    for (i = 0; i < m_app_runinfo.m_reader_numbs; ++i) {

        pdata                               = &pbuf[1 + i*2];
        reader_id                           = *pdata;
        if (reader_id >= m_app_runinfo.m_reader_numbs){
            rsp                             = RSP_INVALID_PARAM;
            goto quit;
        }
        //若所写的阅读器离线  则忽略对此阅读器的写操作
        if (DEV_OFFLINE == pdevice_rfid->reader_status_get(reader_id)){
            continue;
        }
        //设置阅读器id信息
        pdevice_rfid->reader_id_set(reader_id);

        //write to rfid reader
        if (0 != pdevice_rfid->sound_set(pdata[1])){
            rsp                             = RSP_EXEC_FAILURE;
            goto quit;
        }
    }
quit:
    *pbuf++                                 = rsp;
    *plen                                   = 1;

    return (rsp);
}

portBASE_TYPE CApplication::readerrfid_write(uint8 *pbuf, uint16 *plen)
{
    CDevice_Rfid    *pdevice_rfid          = m_app_runinfo.m_pdevice_rfid;
    uint8           rsp                     = RSP_OK;
    uint8           i;
    uint8           reader_id;
    uint8           *pdata;
    uint16          len                     = *plen;
    vector<struct reader_info> *preader_info = pdevice_rfid->reader_info_get();

    if (pbuf[1] != m_app_runinfo.m_reader_numbs){
        rsp                                 = RSP_INVALID_PARAM;
        goto quit;
    }
    //len check
    if (len != (2 + 3*(pbuf[1]))){
        rsp                                 = RSP_INVALID_PARAM_LEN;
        goto quit;
    }

    pbuf[0]                                 &= (TERMINAL_ABILITY_R_DATA|TERMINAL_ABILITY_W_DATA);
    m_app_runinfo.m_ability                 = pbuf[0];
    for (i = 0; i < m_app_runinfo.m_reader_numbs; ++i) {

        pdata                               = &pbuf[2 + i*3];
        reader_id                           = *pdata;
        if (reader_id >= m_app_runinfo.m_reader_numbs){
            rsp                             = RSP_INVALID_PARAM;
            goto quit;
        }
        //若所写的阅读器离线  则忽略对此阅读器的写操作
        if (DEV_OFFLINE == pdevice_rfid->reader_status_get(reader_id)){
            continue;
        }
        //设置阅读器id信息
        pdevice_rfid->reader_id_set(reader_id);

        if (pdata[1] > RFID_READER_MAX_POWER){
            pdata[1]                        = RFID_READER_MAX_POWER;
        }
        if (pdata[2] > RFID_READER_MAX_SCNTIME){
            pdata[2]                        = RFID_READER_MAX_SCNTIME;
        }
        if (pdata[2] < RFID_READER_MIN_SCNTIME){
            pdata[2]                        = RFID_READER_MIN_SCNTIME;
        }
        //write to rfid reader
        if (0 != pdevice_rfid->power_set(pdata[1])){
            rsp                             = RSP_EXEC_FAILURE;
            goto quit;
        }else {
            (*preader_info)[reader_id].m_power        = pdata[1];
        }
        if (0 != pdevice_rfid->querytime_set(pdata[2])){
            rsp                             = RSP_EXEC_FAILURE;
            goto quit;
        }else {
            (*preader_info)[reader_id].m_scntm        = pdata[2];
        }
    }
quit:
    *pbuf++                                 = rsp;
    content_readerinfo_make(pbuf, plen);
    *plen                                   += 1;

    return (rsp);
}

portBASE_TYPE CApplication::containerrfid_r_epc(CDevice_Rfid    *pdevice_rfid)
{
    return pdevice_rfid->query_rfid(&m_app_runinfo.m_epcinfo);
}

//暂未用到
portBASE_TYPE CApplication::containerrfid_w_epc(CDevice_Rfid    *pdevice_rfid, uint8 *pepc)
{
#if 0
    portBASE_TYPE   try_cnt         = 10;
    portBASE_TYPE   rt;
    struct write_info t_writeinfo;

    while (try_cnt > 0){
        t_writeinfo.m_enum          = m_app_runinfo.m_rfidinfo.m_epclen >> 1;
        memcpy((void *)t_writeinfo.m_epcarray, (void *)m_app_runinfo.m_rfidinfo.m_epcarray, m_app_runinfo.m_rfidinfo.m_epclen);
//          t_writeinfo.m_enum  = 12;
//      t_writeinfo.m_epcarray
        t_writeinfo.m_mem           = MEM_EPC;
        t_writeinfo.m_wordptr       = 2;
        t_writeinfo.m_wnum          = 6;

        if ((rt = pdevice_rfid->write_data(&t_writeinfo, pepc))){
            SYS_LOG("-----err:write epc!try_cnt = %d--------\n", try_cnt);
        }
        if (0 == rt){
            break;
        }
        try_cnt--;
    }
    if (try_cnt == 0){
        SYS_LOG("+++++final err:write epc-+++++--\n");
    }

    return ((try_cnt == 0)?(-1):(0));
#endif

    return 0;
}

const uint8     user_region_start_index     = 0;
//unit: word
const uint8     user_region_len             = 6;
enum{
    RECORD_INFO_DATA    = 0x01,
};

portBASE_TYPE CApplication::containerrfid_r_data(CDevice_Rfid   *pdevice_rfid, uint8 index, uint8 *pbuff, uint16 *plen, uint8 ctrl)
{
    struct read_info t_readinfo;
    portBASE_TYPE   rt                  = 0;
    uint8           *buf_tmp            = pbuff;
    uint16          cur_len;

    //read data
    if (0 == CDevice_Rfid::epc_get(&m_app_runinfo.m_epcinfo, index, &t_readinfo.m_enum, t_readinfo.m_epcarray)){
        if (ctrl & RECORD_INFO_DATA){
            //read from user region
            t_readinfo.m_mem    = MEM_USER;
            t_readinfo.m_wordptr = user_region_start_index;
            t_readinfo.m_num    = user_region_len;

            if ((rt = pdevice_rfid->read_data(&t_readinfo, &m_app_runinfo.m_rfidinfo.m_initseq_hi))){
            }
            if (0 == rt){
                LOG_INFO << "-----success:read data--------";
                utils::log_binary_buf("CApplication::containerrfid_r_data",
                        reinterpret_cast<const char *>(&m_app_runinfo.m_rfidinfo.m_initseq_hi),
                        user_region_len<<1);
            }
        }
    }else {
        rt                  = -1;
    }
    if ((0 == rt) && (NULL != pbuff)){
        cur_len                     = sizeof(t_readinfo.m_epcarray);
        *pbuff++                    = t_readinfo.m_enum<<1;
        memcpy(pbuff, &t_readinfo.m_epcarray, cur_len);
        pbuff                       += cur_len;
        if (ctrl & RECORD_INFO_DATA){
            //copy epc info
            cur_len                     = user_region_len<<1;
            *pbuff++                    = cur_len;
            memcpy(pbuff, &m_app_runinfo.m_rfidinfo.m_initseq_hi, cur_len);
            pbuff                       += cur_len;
        }
        if (NULL != plen){
            *plen                                   += pbuff - buf_tmp;
        }
    }

    return (rt);
}

portBASE_TYPE CApplication::containerrfid_w_data(CDevice_Rfid   *pdevice_rfid, uint8 epc_len, uint8 *pepc, uint8 *pdata)
{
    portBASE_TYPE   try_cnt         = 10;
    portBASE_TYPE   rt;
    struct write_info t_writeinfo;

    while (try_cnt > 0){
        t_writeinfo.m_enum          = epc_len >> 1;
        memcpy((void *)t_writeinfo.m_epcarray, (void *)pepc, epc_len);
        t_writeinfo.m_mem           = MEM_USER;
        t_writeinfo.m_wordptr       = user_region_start_index;
        t_writeinfo.m_wnum          = user_region_len;

        if ((rt = pdevice_rfid->write_data(&t_writeinfo, pdata))){
            LOG_WARN << "-----err:write data!try_cnt = [" << try_cnt << "]";
        }
        if (0 == rt){
            break;
        }
        try_cnt--;
    }
    if (try_cnt == 0){
        LOG_ERROR << "-----final err:write data!";
    }

    return (try_cnt == 0)?(-1):(0);
}

uint8 CApplication::protocol_rfid_write(uint8 *pbuf, uint16 len)
{
    CDevice_Rfid    *pdevice_rfid          = m_app_runinfo.m_pdevice_rfid;
    uint8           rsp                     = RSP_OK;
    uint8           data_pos;

    if (pbuf[0] >= m_app_runinfo.m_reader_numbs){
        rsp                                 = RSP_INVALID_PARAM;
        goto quit;
    }
    if (DEV_OFFLINE == pdevice_rfid->reader_status_get(pbuf[0])){
        rsp                                 = RSP_INVALID_PARAM;
        goto quit;
    }
    //1th byte:reader id
    //设置阅读器id信息
    pdevice_rfid->reader_id_set(pbuf[0]);
    //epc len + epc
    //check epc len
    if (pbuf[1] != FIXED_EPC_LEN){
        rsp                                 = RSP_INVALID_PARAM_LEN;
        goto quit;
    }
    //check data len
    data_pos                                = pbuf[1]+2;
    if (pbuf[data_pos]  != FIXED_DATA_LEN){
        rsp                                 = RSP_INVALID_PARAM_LEN;
        goto quit;
    }
    //check reader ability
    if (!(m_app_runinfo.m_ability & TERMINAL_ABILITY_W_DATA)){
        rsp                                 = RSP_ABILITY_ERR;
        goto quit;
    }
    if (0 != containerrfid_w_data(pdevice_rfid, pbuf[1], &pbuf[2], &pbuf[data_pos+1])){
        rsp                                 = RSP_EXEC_FAILURE;
        goto quit;
    }
quit:
    return rsp;
}

portBASE_TYPE CApplication::protocol_rfid_read(void)
{
    CDevice_net     *pdevice_net           = m_app_runinfo.m_pdevice_net;
    CDevice_Rfid    *pdevice_rfid          = m_app_runinfo.m_pdevice_rfid;

    uint8           need_send;
    uint8           i, index, sucess_cnts;
    uint8           reader_id;
    uint8           buffer[1024];
    uint16          len, tot_len_index, tot_len, rfid_total_numb;
    int             rfid_device_online_no;

    rfid_device_online_no                   = pdevice_rfid->rfid_device_online_no_get();
    //format  comm  frame
    //len init
    len                                     = 0;
    need_send                               = 0;
    //reader  numb
    buffer[len++]                           = rfid_device_online_no;
    //record totoal len index  type:uint16
    tot_len_index                           = len;
    len                                     += sizeof(uint16);
    //current len  snapshot
    tot_len                                 = len;
    for (i = 0; i < rfid_device_online_no; ++i) {

        //获取阅读器id信息
        reader_id                           = pdevice_rfid->rfid_device_id_get(i);
        pdevice_rfid->reader_id_set(reader_id);
        //reader  id
        buffer[len++]                       = reader_id;
        //rfid info:   rfid numbs,    epc,   data
        //total rfid numb
        rfid_total_numb                     = len++;
        sucess_cnts                         = 0;
        if (0 == containerrfid_r_epc(pdevice_rfid)){
            for (index = 0; index < m_app_runinfo.m_epcinfo.m_numb; index++){

                if (0 == containerrfid_r_data(pdevice_rfid, index, &buffer[len], &len,
                        (m_app_runinfo.m_ability & TERMINAL_ABILITY_R_DATA)?(RECORD_INFO_DATA):(0))){
                    sucess_cnts++;
                    need_send               = 1;
                }
            }
        }else {
            //no rfids exist
        }

        //write rfid total numb
        buffer[rfid_total_numb]             = sucess_cnts;
    }
    if (1 == need_send){
        //caculate tot
        tot_len                             = len - tot_len;
        //write tot len to buffer
        ST_WORD(&buffer[tot_len_index], tot_len);
        pdevice_net->package_send_rfid((char *)buffer, len);
    }else {
        //可通过select等相关函数睡眠会
//        msleep(50);
    }

    return 0;
}

portBASE_TYPE CApplication::device_status_send(void)
{
    CDevice_net     *pdevice_net            = m_app_runinfo.m_pdevice_net;
    CDevice_Rfid    *pdevice_rfid           = m_app_runinfo.m_pdevice_rfid;
    uint8           loop;
    uint8           id;
    uint8           buff[10];
    portBASE_TYPE   rt;
    uint16          len                     = 0;
    uint16          i                       = 0;
    vector<struct reader_info> *preader_info = pdevice_rfid->reader_info_get();

    //此处要对处于dev_offline状态下的设备做一次扫描  确定其是否在线  注意此次扫描的等待时间应该缩短  扫描
    //结束后 再将等待时间恢复
    //通道电源打开时 扫描阅读器
    if (pdevice_rfid->channel_power_get()){
        for (i = 0; i < m_app_runinfo.m_reader_numbs; ++i) {
            id                                  = (*preader_info)[i].m_id_;
            if (DEV_OFFLINE == pdevice_rfid->reader_status_get(id)){
                //巡查设备是否在线
                pdevice_rfid->reader_id_set(id);
                pdevice_rfid->max_wait_time_restore();
                //发出巡查命令  让设备更新自身在线、离线状态
                pdevice_rfid->query_readerinfo(NULL);
            }
        }
    }

    //get mode stat
    buff[len++]                             = m_app_runinfo.m_mode;
    //get channel power stat
    buff[len++]                             = pdevice_rfid->channel_power_get();
    //get ability stat
    buff[len++]                             = m_app_runinfo.m_ability;
    //get devices stat
    buff[len++]                             = m_app_runinfo.m_reader_numbs;
    for (i = 0; i < m_app_runinfo.m_reader_numbs; ++i) {
        id                                  = (*preader_info)[i].m_id_;
        buff[len++]                         = id;
        buff[len++]                         = pdevice_rfid->reader_status_get(id);
        buff[len++]                         = (*preader_info)[i].m_power;
        buff[len++]                         = (*preader_info)[i].m_scntm;
    }
    loop                                    = 0;
    do{
        LOG_INFO << "send device stat info to host";
        rt                                  = pdevice_net->package_send_status((char *)buff, len);
        if (rt != 0){

        }else {
            break;
        }
    } while(loop);

    return 0;
}

bool CApplication::timer_timeout_occured(void)
{
    uint64_t howmany;
    ssize_t n = ::read(m_app_runinfo.timer_fd_, &howmany, sizeof howmany);

    if (n < 0){
        return false;
    }
    if (n != sizeof(howmany))
    {
        LOG_ERROR << "timerfd::read() reads " << n << " bytes instead of 8";
    }

    return true;
}

portBASE_TYPE CApplication::readerrfid_channelpower_set(uint8 *pbuf, uint16 *plen)
{
    CDevice_Rfid    *pdevice_rfid           = m_app_runinfo.m_pdevice_rfid;
	channel         *pchannel               = pdevice_rfid->channel_get();

	if (pbuf[0]){
        pchannel->power_on();
	}else {
        pchannel->power_off();
	}
	readerrfid_channelpower_get(pbuf, plen);

    return RSP_OK;
}

portBASE_TYPE CApplication::readerrfid_channelpower_get(uint8 *pbuf, uint16 *plen)
{
    CDevice_Rfid    *pdevice_rfid           = m_app_runinfo.m_pdevice_rfid;

    *plen                                   = 2;
    *pbuf++                                 = RSP_OK;
    *pbuf++                                 = pdevice_rfid->channel_power_get();

    return RSP_OK;
}

portBASE_TYPE CApplication::package_event_handler(frame_ctl_t *pframe_ctl, uint8 *pbuf, uint16 len)
{
    CDevice_net *pdevice_net                    = m_app_runinfo.m_pdevice_net;
    uint8        rsp;
    uint8        fliter                         = 0;
    uint8        func_code                      = pframe_ctl->app_frm_ptr.fun;

    LOG_TRACE << "a net frame receievd, call CApplication::package_event_handler to handle; func_code = ["
            << func_code << "]";
    //主动模式下
    if (m_app_runinfo.m_mode == MODE_INITIATIVE){
//      if (pframe_ctl->mac_frm_ptr.ctl.ack_mask == 1){
//          rsp                 = RSP_ACK_IN_ACTIVE_MODE;
//            fliter              = 1;
//      }else if(func_code != def_FUNC_CODE_MODE_SET){
//            fliter              = 1;
//          rsp                 = RSP_EXEC_FAILURE;
//      }
        //接收到应答帧  直接丢弃
        if (pframe_ctl->mac_frm_ptr.ctl.ack_mask == 1){
            LOG_WARN << "Net: recvice ack frame in inactive mode";
            return 0;
        //主动模式下 只响应模式设置命令
        }else if(func_code != def_FUNC_CODE_MODE_SET){
            fliter                              = 1;
            rsp                                 = RSP_EXEC_FAILURE;
        }
        if (1 == fliter){
            pdevice_net->package_send_rsp(func_code, &rsp, sizeof(rsp));
            return 0;
        }
    }
    switch (func_code) {
        case def_FUNC_CODE_MODE_SET:
            if (pbuf[0] > MODE_PASSIVITY){
                rsp                             = RSP_INVALID_PARAM;
            }else {
                rsp                             = RSP_OK;
            }
            if (rsp == RSP_OK){
                m_app_runinfo.m_mode            = pbuf[0];
            }
            pdevice_net->package_send_rsp(pframe_ctl->app_frm_ptr.fun, &rsp, sizeof(rsp));
            break;
        case def_FUNC_CODE_READER_QUERY:
            pbuf[0]                             = RSP_OK;
            readerrfid_query(&pbuf[1], &len);
            len++;
            utils::log_binary_buf_trace("def_FUNC_CODE_READER_QUERY",
                    reinterpret_cast<const char *>(pbuf), len);
            pdevice_net->package_send_rsp(pframe_ctl->app_frm_ptr.fun, pbuf, len);
            break;

        case def_FUNC_CODE_READER_SET:
            readerrfid_write(pbuf, &len);
            pdevice_net->package_send_rsp(pframe_ctl->app_frm_ptr.fun, pbuf, len);
            break;
        case def_FUNC_CODE_RFID_W:
            rsp                                 = protocol_rfid_write(pbuf, len);
            pdevice_net->package_send_rsp(pframe_ctl->app_frm_ptr.fun, &rsp, sizeof(rsp));
            break;
        case def_FUNC_CODE_SOUND_SET:
            readerrfid_sound_set(pbuf, &len);
            pdevice_net->package_send_rsp(pframe_ctl->app_frm_ptr.fun, pbuf, len);
            break;

        case def_FUNC_CODE_CHANNEL_POWER_SET:
            readerrfid_channelpower_set(pbuf, &len);
            pdevice_net->package_send_rsp(pframe_ctl->app_frm_ptr.fun, pbuf, len);
            break;

        case def_FUNC_CODE_CHANNEL_POWER_GET:
            readerrfid_channelpower_get(pbuf, &len);
            pdevice_net->package_send_rsp(pframe_ctl->app_frm_ptr.fun, pbuf, len);
            break;
 
        default:
            rsp                                  = RSP_INVALID_CMD;
            pdevice_net->package_send_rsp(func_code, &rsp, sizeof(rsp));
            break;
    }


    return 0;
}

void CApplication::quit(void)
{
    //遍历容器
    for (boost::ptr_vector<channel>::iterator iter = m_app_runinfo.channel_vector_.begin();
            iter != m_app_runinfo.channel_vector_.end(); ++iter){
        //关闭通道电源
        iter->power_off();
    }
}

portBASE_TYPE CApplication::run()
{
    CDevice_net     *pdevice_net                    = m_app_runinfo.m_pdevice_net;
    CDevice_Rfid    *pdevice_rfid                   = m_app_runinfo.m_pdevice_rfid;
    pid_t           parent_pid;

    while(m_app_runinfo.m_status != enum_APP_STATUS_EXIT){
        switch (m_app_runinfo.m_status){
        case enum_APP_STATUS_INIT:

            m_app_runinfo.m_status                  = enum_APP_STATUS_SEND_READERINFO;
            break;

        //初始化错误  进程不做任何事情 只是检查父进程是否退出
        case enum_APP_STATUS_INIT_ERR:

            LOG_ERROR << "sleep 2 and Check whether the parent process exist";
            CurrentThread::sleepUsec(2*1000*1000);
            break;

        case enum_APP_STATUS_SEND_READERINFO:
        {
            if (0 == readerrfid_init()){
                m_app_runinfo.m_status      = enum_APP_STATUS_RUN;
            }
        }
            break;

        case enum_APP_STATUS_RUN:
            //主动模式下
            if (m_app_runinfo.m_mode == MODE_INITIATIVE){
                //通道电源打开时 扫描阅读器
                if (pdevice_rfid->channel_power_get()){
                    //此函数内部有进程调度点
                    protocol_rfid_read();
                }else {
                    //延迟1s  让当前进程休眠  避免busy_loop
                    CurrentThread::sleepUsec(1*1000*1000);
                }
            }else {

            }
            if (timer_timeout_occured()){
                device_status_send();
            }
            pdevice_net->package_event_fetch();
            break;

        default:
            break;
        }
        //判断父进程是否存在
        parent_pid                          = ::getppid();
        if ((parent_pid == 1) || (parent_pid != m_app_runinfo.ppid_)){
            LOG_WARN << "parent process exit, close channel power and send sigkill to myself";
            quit();
            ::raise (SIGKILL);
        }
    }

    return 0;
}

int main(int argc, char**argv)
{
	CApplication  *pcapplication;
	char    log_file_path[150]    = "which ";
	const char *pbase_name       = ::basename(argv[0]);
	char    *ptmp;
	FILE    *stream;

#if (def_DBG_IN_PC_SINGLE > 0)
	argv[1]                       = (char *)"/home/barnard/work/commu_manager/manager/config/config.xml";
	strcpy(log_file_path, PROCESS_PREFIX_PATH);
	strcat(log_file_path, "log/");
	strcat(log_file_path, pbase_name);
#else
	if (argc != 2){
		LOG_SYSFATAL << "argc must = 2" << ::getpid();
	}
    strcat(log_file_path, pbase_name);
	stream                   = popen(log_file_path, "r" );
	if (NULL == stream){
		LOG_SYSFATAL << "popen failed!";
	}
	memset(log_file_path, '\0', sizeof(log_file_path));
	fread(log_file_path, sizeof(char), sizeof(log_file_path),  stream);
	pclose(stream);

	ptmp                      = strrchr(log_file_path, '/');
	strcpy(ptmp, "/../log/");
	strcat(log_file_path, pbase_name);
#endif

	pcapplication                   = CApplication::GetInstance();
    pcapplication->init(log_file_path, argv[1]);
    pcapplication->run();

	LOG_INFO << "program exit";
	//删除共享内存
	t_project_datum.shmem_.detach();
    t_project_datum.shmem_.destroy();
}



