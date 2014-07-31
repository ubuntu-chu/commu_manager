#include <datum.h>
#include "zygote.h"
#include <parse.h>
#include <utils.h>
#include <sys/wait.h>


using std::string;
using std::pair;

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
        zygote::GetInstance()->quit();
    }
}

//---------------------------------------------------------------

zygote    *zygote::m_pzygote = NULL;
class project_datum  t_project_datum;

static zygote     c_zygote;

zygote *zygote::GetInstance(void)
{

    if (NULL == m_pzygote){
        m_pzygote         = &c_zygote;
    }

    return (m_pzygote);
}


portBASE_TYPE zygote::init(const char *log_file_path, const char *config_file_path)
{
	std::string      process_name_str;

	m_app_runinfo.m_status                  = enum_APP_STATUS_INIT;
    utils::signal_handler_install(SIGINT, signal_handle);

    //获取进程名字
    process_name_str                        = ProcessInfo::procname();

#if 1
    //设置日志文件名称
    g_logFile.reset(new muduo::LogFile(log_file_path, 200 * 1000));
    muduo::Logger::setOutput(outputFunc);
    muduo::Logger::setFlush(flushFunc);
#endif

	LOG_INFO << "project xml config file parse";
	if (xml_parse(config_file_path)){
		LOG_SYSFATAL << "project xml config file parse failed!";
	}
    Logger::setLogLevel(static_cast<muduo::Logger::LogLevel>(t_project_datum.pproject_config_->log_lev_get()));
//    Logger::setLogLevel(muduo::Logger::INFO);

    return 0;
}

portBASE_TYPE zygote::run()
{
	m_app_runinfo.m_status                  = enum_APP_STATUS_RUN;
	project_config	*pproject_config        = t_project_datum.pproject_config_;
	process_config  &process_conf	        = pproject_config->process_config_get();
	process_node    *process_node;
	int             process_vector_no       = process_conf.process_vector_no_get();
	int             i, ret, status;
	const  char    *path;
    pid_t pid;

	m_app_runinfo.vec_pid_.clear();
    while(m_app_runinfo.m_status == enum_APP_STATUS_RUN){
        //依据配置文件创建进程
        for (i = 0; i < process_vector_no; i++){
            process_node                    = process_conf.process_node_get(i);
            path                            = process_node->file_path_get();

            pid                             = fork();
            if (pid == -1) {
                LOG_WARN << "fork() err. errno:[" << errno << "] error msg:<" << strerror(errno) << "<";
            }
            //子进程
            if (pid == 0) {
                ret                         = execlp(path, path, (char *)0);
                if (ret < 0) {
                    LOG_ERROR << "execu ret:[" << ret << "] errno:["<< errno
                            << "] error msg:<" << strerror(errno) << "<";
                    continue;
                }
                exit(0);
            }

            LOG_INFO << "fork to exec subprocess name:[" << process_node->name_get()
                    << "] path:<" << process_node->file_path_get() << ">";
            //父进程
            m_app_runinfo.vec_pid_.push_back(pid);
            m_app_runinfo.map_pid_.insert(pair<pid_t, const char*>(pid, path));
        }

        LOG_INFO << "zygote wait begin";
        if (pid > 0) {
            pid = wait(&status);
            LOG_INFO << "wait return";
        }

    }

    return 0;
}

void zygote::quit(void)
{
    m_app_runinfo.m_status  = enum_APP_STATUS_EXIT;
}

int main(int argc, char**argv)
{
	const char *config_file_path = "/home/barnard/work/commu_manager/manager/config/config.xml";
	zygote  *pzygote;

	argc 			= 2;

#if 0
	config_file_path 	= argv[1];
#endif
	if (argc != 2){
		LOG_SYSFATAL << "argc must = 2" << getpid();
	}

	pzygote                   = zygote::GetInstance();
    pzygote->init(::basename(argv[0]), config_file_path);
    pzygote->run();

	LOG_INFO << "program exit";
	//删除共享内存
	t_project_datum.shmem_.detach();
}



