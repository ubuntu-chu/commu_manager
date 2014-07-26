#include <sys/timerfd.h>  /**/
#include <config.h>
#include "parse.h"
#include "datum.h"
#include "channel/channel.h"
#include "./io_node/ext_client.h"
#include "./io_node/inner_client.h"
#include "./io_node/inner_server.h"
#include "rfid.h"
#include "net.h"
#include <protocol_rfid.h>
#include "application.h"

#define         DEV_ONLINE                              (1)
#define         DEV_OFFLINE                             (0)

using std::string;

boost::scoped_ptr<muduo::LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
  g_logFile->append(msg, len);
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

//---------------------------------------------------------------

CApplication    *CApplication::m_pcapplicaiton = NULL;

CApplication *CApplication::GetInstance(void)
{
    static CApplication     c_application;

    if (NULL == m_pcapplicaiton){
        m_pcapplicaiton         = &c_application;
    }

    return (m_pcapplicaiton);
}

static  CDevice_Rfid       s_Device_rfid;
static  CDevice_net        s_Device_net;


portBASE_TYPE CApplication::init(const char *config_file_path)
{
	struct sigaction action;
	std::string      process_name_str;

	m_app_runinfo.m_status                  = enum_APP_STATUS_INIT;
	m_app_runinfo.m_pdevice_rfid            = &s_Device_rfid;
	m_app_runinfo.m_pdevice_net             = &s_Device_net;
	m_app_runinfo.m_ability                 = TERMINAL_ABILITY_NONE;
    m_app_runinfo.m_mode                    = MODE_INITIATIVE;

	action.sa_handler                       = signal_handle;
    sigemptyset(&action.sa_mask);
    action.sa_flags                         = 0;
    sigaction(SIGINT, &action, 0);

    Logger::setLogLevel(Logger::TRACE);
    //获取进程名字
    process_name_str                        = ProcessInfo::procname();

#if 0
    //设置日志文件名称
    g_logFile.reset(new muduo::LogFile(::basename(argv[0]), 200 * 1000));
    muduo::Logger::setOutput(outputFunc);
    muduo::Logger::setFlush(flushFunc);
#endif

	LOG_INFO << "project xml config file parse";
	if (xml_parse(config_file_path)){
		LOG_SYSFATAL << "project xml config file parse failed!";
	}

	//依据配置资源 创建channel
	project_config	*pproject_config     = t_project_datum.pproject_config_;
	io_config       &io_conf	        = pproject_config->io_config_get();
//    boost::ptr_vector<channel> channel_vector;
//    vector<boost::shared_ptr<channel> > channel_vector;
	io_node         *pio_node;
	int             i, j;
	int             io_vector_no;

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
                //开启此通道电源
                pchannel->power_on();
                if (pchannel->contain_protocol(def_PROTOCOL_MAC_NAME)){

                    m_app_runinfo.m_pdevice_net->channel_set(pchannel);
                }
            }
        }
    }
#if 0
    //遍历容器
    for(vecotr<boost::shared_ptr<channel> >::iterator iter = channel_vector.begin();
//    for(boost::ptr_vector<channel>::iterator iter = channel_vector.begin();
            iter != channel_vector.end(); ++iter){
        if (iter->contain_protocol(def_PROTOCOL_RFID_NAME)){

            device_rfid.channel_set(iter.get());
        }

    }
#endif

    //创建定时器  此定时器在多线程环境中使用  因此不能使用setitime 及 timer_create 此定时器仅用于查询1s是否到时
    m_app_runinfo.timer_fd_                 = ::timerfd_create(CLOCK_MONOTONIC,
                                                       TFD_NONBLOCK | TFD_CLOEXEC);
    if (m_app_runinfo.timer_fd_ < 0)
    {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
	//设定1s循环定时
    struct itimerspec new_value;

    new_value.it_value.tv_sec               = 1;
    new_value.it_value.tv_nsec              = 0;
    new_value.it_interval.tv_sec            = 1;
    new_value.it_interval.tv_nsec           = 0;

    if (::timerfd_settime(m_app_runinfo.timer_fd_, 0, &new_value, NULL) == -1){
        LOG_SYSERR << "timerfd_settime()";
    }

    return 0;
}

void CApplication::content_readerinfo_make(uint8 *pbuf, uint16 *plen)
{
    uint16          len                     = 0;
    uint8           i                       = 0;

    if (NULL == pbuf){
        return;
    }
    //capacity
    pbuf[len++]                         = m_app_runinfo.m_ability;
    //reader numbs
    pbuf[len++]                         = m_app_runinfo.m_reader_numbs;
    for (i = 0; i < m_app_runinfo.m_reader_numbs; ++i) {
        pbuf[len++]                     = m_app_runinfo.m_readerinfo_vec_[i].m_id_;
        pbuf[len++]                     = m_app_runinfo.m_readerinfo_vec_[i].m_power;
        pbuf[len++]                     = m_app_runinfo.m_readerinfo_vec_[i].m_scntm;
    }
    if (NULL != plen){
        *plen                           = len;
    }
}

portBASE_TYPE CApplication::readerrfid_init(void)
{
    CDevice_net     *pdevice_net           = m_app_runinfo.m_pdevice_net;
    CDevice_Rfid    *pdevice_rfid          = m_app_runinfo.m_pdevice_rfid;
    uint8           loop                   = 3;
    uint8           buffer[500];
    portBASE_TYPE   rt;
    uint16          len                    = 0;
    list_head_t     *pdevice_list_head;
    list_node_t     *pos;
    device_node     *pdevice_node;
    class device_rfid_reader_node   *pnode_rfid_reader;
    struct reader_info  t_reader_info;

    //获取通道下所挂接设备数量

    pdevice_list_head                       = pdevice_rfid->device_list_head_get();

    while (loop){

        //设备在线数量
        m_app_runinfo.m_reader_online_numbs = 0;
        m_app_runinfo.m_reader_numbs        = 0;
        m_app_runinfo.m_readerinfo_vec_.clear();
        //遍历设备链表
        list_for_each(pos, pdevice_list_head){
            pdevice_node        = device_node::device_entry(pos);
            pnode_rfid_reader = reinterpret_cast<device_rfid_reader_node *>(pdevice_node);

            //查询设备信息
            t_reader_info.m_id_      = pnode_rfid_reader->id_get();
            pdevice_rfid->reader_id_set(t_reader_info.m_id_);
            rt              = pdevice_rfid->query_readerinfo(&t_reader_info);
            //init reader query time, power
            if (rt == 0){
                t_reader_info.m_exist_                  = DEV_ONLINE;
                m_app_runinfo.m_reader_online_numbs++;
                //设置读写时最大等待时间  此时间是指程序读时等待阅读器反应时间
                pdevice_rfid->max_wait_time_set(t_reader_info.m_scntm/10);
            }else {
                t_reader_info.m_exist_                  = DEV_OFFLINE;
            }
            m_app_runinfo.m_reader_numbs++;
            m_app_runinfo.m_readerinfo_vec_.push_back(t_reader_info);
        }
        LOG_INFO << "rfid device config no [" << m_app_runinfo.m_reader_numbs
                << "]; exist no [" << m_app_runinfo.m_reader_online_numbs << "]";
        //when no reader exist, reader_numbs = 0
        //send reader info  to host
        content_readerinfo_make(buffer, &len);
        rt     = pdevice_net->package_send_readerinfo((char *)buffer, len);

        LOG_INFO << "pdevice_net->package_send_readerinfo with loop = [" << loop << "]";
        utils::log_binary_buf("CApplication::readerrfid_init",
                reinterpret_cast<const char *>(buffer), len);

        if (0 == m_app_runinfo.m_reader_online_numbs){
            LOG_INFO << "err:no rfid reader find!";
            continue;
        }
        if (rt != 0){
            sleep(1);
            loop--;
        }else {
            break;
        }
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

portBASE_TYPE CApplication::readerrfid_write(uint8 *pbuf, uint16 *plen)
{

    return 0;
}

portBASE_TYPE CApplication::containerrfid_r_epc(CDevice_Rfid    *pdevice_rfid)
{
    return pdevice_rfid->query_rfid(&m_app_runinfo.m_epcinfo);
}

portBASE_TYPE CApplication::containerrfid_w_epc(CDevice_Rfid    *pdevice_rfid, uint8 *pepc)
{

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

    return 0;
}

uint8 CApplication::protocol_rfid_write(uint8 *pbuf, uint16 len)
{

    return 0;
}

int CApplication::rfid_device_id_get(int index)
{
    vector<struct reader_info>::iterator it;
    vector<struct reader_info>::iterator end    = m_app_runinfo.m_readerinfo_vec_.end();
    int i;

    for (it = m_app_runinfo.m_readerinfo_vec_.begin(); it != end; ++it){
        if (it->m_exist_ == DEV_ONLINE){
            if (i == index){
                break;
            }
            i++;
        }
    }

    return it->m_id_;
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

    //format  comm  frame
    //len init
    len                                     = 0;
    need_send                               = 0;
    //reader  numb
    buffer[len++]                           = m_app_runinfo.m_reader_online_numbs;
    //record totoal len index  type:uint16
    tot_len_index                           = len;
    len                                     += sizeof(uint16);
    //current len  snapshot
    tot_len                                 = len;
    for (i = 0; i < m_app_runinfo.m_reader_online_numbs; ++i) {

        //获取阅读器id信息
        reader_id                           = rfid_device_id_get(i);
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


portBASE_TYPE CApplication::run()
{
    CDevice_net *pdevice_net                = m_app_runinfo.m_pdevice_net;

	m_app_runinfo.m_status                  = enum_APP_STATUS_RUN;
    readerrfid_init();

    while(m_app_runinfo.m_status == enum_APP_STATUS_RUN){
        if (m_app_runinfo.m_mode == MODE_INITIATIVE){

            protocol_rfid_read();
            if (timer_timeout_occured()){
                device_status_send();
                LOG_INFO << "device status send called";
            }
        }else {

        }
//        pdevice_net->package_event_fetch();
    }

    return 0;
}

int main(int argc, char**argv)
{
	const char *config_file_path = "/home/barnard/work/commu_manager/manager/config/config.xml";
	CApplication  *pcapplication;

	argc 			= 2;

#if 0
	config_file_path 	= argv[1];
#endif
	if (argc != 2){
		LOG_SYSFATAL << "argc must = 2" << getpid();
	}

	pcapplication                   = CApplication::GetInstance();
    pcapplication->init(config_file_path);
    pcapplication->run();

//    device_rfid.reader_id_set(0);
	LOG_INFO << "program exit";
	//删除共享内存
	t_project_datum.shmem_.detach();
}



