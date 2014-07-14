#include <includes/includes.h>
#include <includes/config.h>
#include "parse.h"
#include "datum.h"
#include "./io_node/ext_client.h"
#include "./io_node/inner_client.h"
#include "./io_node/inner_server.h"

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
        t_project_datum.pevent_loop_->quit();
    }
}

int main(int argc, char**argv)
{
	const char *config_file_path = "/home/barnard/work/commu_manager/manager/config/config.xml";
	argc 			= 2;
	struct sigaction action;
	std::string      process_name_str;
	boost::ptr_vector<class ext_client> ext_client_vector;
	boost::ptr_vector<class inner_client> inner_client_vector;
	boost::ptr_vector<class inner_server> inner_server_vector;
//	boost::ptr_vector<class ext_s> ext_client_vector;


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

	EventLoop loop;
	t_project_datum.pevent_loop_             = &loop;

	//依据配置资源 创建客户端
	project_config	*pproject_config     = t_project_datum.pproject_config_;
	io_config       &io_conf	        = pproject_config->io_config_get();
	io_node         *pio_node;
	int             i, j;
	int             io_vector_no, io_type;

    for (i = io_conf.io_type_start(); i < io_conf.io_type_end(); i++){
        io_vector_no                    = io_conf.io_vector_no_get(i);
        for (j = 0; j < io_vector_no; j++){
            pio_node                    = io_conf.io_vector_get(i, j);
#if 0
            //查找io配置中属于当前进程的io_node
            if (0 == strcmp(process_name_str.c_str(), pio_node->process_get())){
                io_type                 = io_node::io_type_get(pio_node->type_get());
                switch (io_type){
                case IO_TYPE_EXT_CLIENT:
                {
                    io_tcp_ext_client_node *pio_tcp_ext_client_node =
                            reinterpret_cast<io_tcp_ext_client_node *>(pio_node);
                    const InetAddress serverAddr(pio_tcp_ext_client_node->server_ip_get(),
                            pio_tcp_ext_client_node->server_port_get());

                    ext_client_vector.push_back(new ext_client(&loop, serverAddr,
                            pio_tcp_ext_client_node->name_get()));
                    LOG_INFO << "IO_TYPE_EXT_CLIENT create; serverip = " << serverAddr.toIpPort()\
                            << " local ip = " << pio_tcp_ext_client_node->client_ip_get();
                    ext_client_vector.rbegin()->connect();
                }
                    break;

                case IO_TYPE_INNER_CLIENT:
                {
                    io_tcp_client_node *pio_tcp_client_node =
                            reinterpret_cast<io_tcp_client_node *>(pio_node);
                    const InetAddress serverAddr(pio_tcp_client_node->server_ip_get(),
                            pio_tcp_client_node->server_port_get());

                    inner_client_vector.push_back(new inner_client(&loop, serverAddr,
                            pio_tcp_client_node->name_get()));
                    LOG_INFO << "IO_TYPE_INNER_CLIENT create; serverip = " << serverAddr.toIpPort()\
                            << " local ip = " << pio_tcp_client_node->client_ip_get();
                    inner_client_vector.rbegin()->connect();
                }
                    break;

                case IO_TYPE_INNER_SERVER:
                {
                    io_tcp_server_node *pio_tcp_server_node =
                            reinterpret_cast<io_tcp_server_node *>(pio_node);
                    const InetAddress serverAddr(pio_tcp_server_node->server_ip_get(),
                            pio_tcp_server_node->server_port_get());

                    inner_server_vector.push_back(new inner_server(&loop, serverAddr,
                            pio_tcp_server_node->name_get()));
                    LOG_INFO << "IO_TYPE_INNER_SERVER create; serverip = " << serverAddr.toIpPort();
                    inner_server_vector.rbegin()->start();
                }
                    break;

                case IO_TYPE_EXT_COM:

                    break;
                }
            }
#endif
        }
    }

	LOG_INFO << "loop.loop()";
	loop.loop();

	LOG_INFO << "program exit";
	t_project_datum.shmem_.detach();
}

