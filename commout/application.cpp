#include <includes/includes.h>
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
#include "commout.h"

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

	m_app_runinfo.m_pdevice_rfid            = &s_Device_rfid;
	m_app_runinfo.m_pdevice_net             = &s_Device_net;

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
    boost::ptr_vector<channel> channel_vector;
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
                channel_vector.push_back(pchannel);
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

    return 0;
}

void CApplication::content_readerinfo_make(uint8 *pbuf, uint16 *plen)
{
    uint16          len                     = 0;
    uint8           i                       = 0;
    CDevice_Rfid    *pdevice_rfid;

    if (NULL == pbuf){
        return;
    }
    //capacity
    pbuf[len++]                         = m_app_runinfo.m_ability;
    //reader numbs
    pbuf[len++]                         = m_app_runinfo.m_reader_numbs;
    for (i = 0; i < m_app_runinfo.m_reader_numbs; ++i) {
//        pbuf[len++]                     = s_rfid_reader_id_array[i];
        pbuf[len++]                     = m_app_runinfo.m_readerinfo[i].m_power;
        pbuf[len++]                     = m_app_runinfo.m_readerinfo[i].m_scntm;
//        pdevice_rfid                    = (CDevice_Rfid *)rfid_device_get(i);
        //set max wait time
//        pdevice_rfid->max_wait_time_set(m_app_runinfo.m_readerinfo[i].m_scntm/10);
    }
    if (NULL != plen){
        *plen                           = len;
    }
}

portBASE_TYPE CApplication::readerrfid_init(void)
{

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

    return 0;
}

portBASE_TYPE CApplication::containerrfid_w_data(CDevice_Rfid   *pdevice_rfid, uint8 epc_len, uint8 *pepc, uint8 *pdata)
{

    return 0;
}

uint8 CApplication::protocol_rfid_write(uint8 *pbuf, uint16 len)
{

    return 0;
}

portBASE_TYPE CApplication::protocol_rfid_read(void)
{

    return 0;
}

portBASE_TYPE CApplication::device_status_send(void)
{

    return 0;
}

portBASE_TYPE CApplication::run()
{
    CDevice_net      *pdevice_net             = m_app_runinfo.m_pdevice_net;

//    readerrfid_init();

    while(1){
        if (m_app_runinfo.m_mode == MODE_INITIATIVE){

            protocol_rfid_read();
#if 0
            if (cpu_timetrig_1s()){
                uint8 buffer[100];

                device_status_send();
            }
#endif
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



