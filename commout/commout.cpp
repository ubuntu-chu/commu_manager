#include <includes/includes.h>
#include <config.h>
#include "parse.h"
#include "datum.h"
#include "channel/channel.h"
#include "./io_node/ext_client.h"
#include "./io_node/inner_client.h"
#include "./io_node/inner_server.h"
#include "rfid.h"

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

static CDevice_Rfid device_rfid;

int main(int argc, char**argv)
{
	const char *config_file_path = "/home/barnard/work/commu_manager/manager/config/config.xml";
	argc 			= 2;
	struct sigaction action;
	std::string      process_name_str;


#if 0
	config_file_path 	= argv[1];
#endif
	if (argc != 2){
		LOG_SYSFATAL << "argc must = 2" << getpid();
	}
	action.sa_handler       = signal_handle;
    sigemptyset(&action.sa_mask);
    action.sa_flags         = 0;
    sigaction(SIGINT, &action, 0);

    Logger::setLogLevel(Logger::TRACE);


    //设置进程名字
    process_name_str            = ProcessInfo::procname();

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

//	EventLoop loop;
//	t_project_datum.pevent_loop_             = &loop;

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
                if (pchannel->contain_protocol("rfid")){

                    device_rfid.channel_set(pchannel);
                }
            }
        }
    }
#if 0
    //遍历容器
    for(vecotr<boost::shared_ptr<channel> >::iterator iter = channel_vector.begin();
//    for(boost::ptr_vector<channel>::iterator iter = channel_vector.begin();
            iter != channel_vector.end(); ++iter){
        if (iter->contain_protocol("rfid")){

            device_rfid.channel_set(iter.get());
        }

    }
#endif
    device_rfid.reader_id_set(0);
//    device_rfid.query_rfid(NULL);
    device_rfid.query_readerinfo(NULL);

//	LOG_INFO << "loop.loop()";
//	loop.loop();
//    while (1);
    while (1){
        sleep(1);
        device_rfid.query_readerinfo(NULL);
    }

	LOG_INFO << "program exit";
	t_project_datum.shmem_.detach();
}

