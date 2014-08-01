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
	m_app_runinfo.config_file_path_         = config_file_path;
//    utils::signal_handler_install(SIGINT, signal_handle);

    //获取进程名字
    process_name_str                        = ProcessInfo::procname();

#if 1
    //设置日志文件名称
    g_logFile.reset(new muduo::LogFile(log_file_path, 200 * 1000));
    muduo::Logger::setOutput(outputFunc);
    muduo::Logger::setFlush(flushFunc);
#endif

	LOG_INFO << "parse project xml config file: " << config_file_path;
	if (xml_parse(config_file_path, &t_project_datum.project_config_)){
		LOG_SYSFATAL << "project xml config file parse failed!";
	}
    Logger::setLogLevel(static_cast<muduo::Logger::LogLevel>(t_project_datum.project_config_.log_lev_get()));
//    Logger::setLogLevel(muduo::Logger::INFO);

    return 0;
}

void zygote::exit_code_analyze(pid_t pid, int status)
{
    if (-1 == pid){
        LOG_WARN << "Failed to wait for child. errno:[" << errno << "] error msg:<" << strerror(errno) << "<";
    }else if (WIFEXITED(status)){
        LOG_WARN << "pid = [" << pid << "] path:<" << m_app_runinfo.map_pid_[pid]->file_path_get()
                 << "> terminated normally, exit status = [" << WEXITSTATUS(status) << "]";
    }else if (WIFSIGNALED(status)){
        LOG_WARN << "pid = [" << pid << "] path:<" << m_app_runinfo.map_pid_[pid]->file_path_get()
                 << "> terminated abnormal due to uncaught signal = [" << WTERMSIG(status) << "]";
    }else if (WIFSTOPPED(status)){
        LOG_WARN << "pid = [" << pid << "] path:<" << m_app_runinfo.map_pid_[pid]->file_path_get()
                 << "> child stopped due to signal = [" << WSTOPSIG(status) << "]";
    }
}

pid_t zygote::fork_subproc(const char *path, char *const argv[])
{
    pid_t pid;

    pid                             = fork();
    if (pid == -1) {
        LOG_WARN << "fork() err. errno:[" << errno << "] error msg:<" << strerror(errno) << ">";
        exit(1);
    }
    //子进程
    if (pid == 0) {
        int ret                             = execvp(path, argv);
        if (ret < 0) {
            LOG_ERROR << "execu ret:[" << ret << "] errno:["<< errno
                    << "] error msg:<" << strerror(errno) << ">";
        }
        exit(0);
    }

    return pid;
}

portBASE_TYPE zygote::run()
{
	m_app_runinfo.m_status                  = enum_APP_STATUS_RUN;
//	project_config	*pproject_config        = t_project_datum.pproject_config_;
	project_config	*pproject_config        = &t_project_datum.project_config_;
	process_config  &process_conf	        = pproject_config->process_config_get();
	process_node    *pprocess_node;
	int             process_vector_no       = process_conf.process_vector_no_get();
	int             i, status;
    pid_t pid;
    map<pid_t, process_node *>::iterator it;
    const char *child_argv[100] = {0};


	m_app_runinfo.map_pid_.clear();
    while(m_app_runinfo.m_status == enum_APP_STATUS_RUN){
        if (0 == process_vector_no){
            LOG_ERROR << "project xml config file error! none process want to fork to exec";
        }
        //依据配置文件创建进程
        for (i = 0; i < process_vector_no; i++){
            pprocess_node                   = process_conf.process_node_get(i);

            child_argv[0]                   = pprocess_node->file_path_get();
            child_argv[1]                   = m_app_runinfo.config_file_path_;
            pid                             = fork_subproc(child_argv[0], (char **)child_argv);

            LOG_INFO << "fork to exec subprocess name:[" << pprocess_node->name_get()
                    << "] path:<" << pprocess_node->file_path_get() << ">";
            m_app_runinfo.map_pid_.insert(pair<pid_t, process_node*>(pid, pprocess_node));
        }

        while (1){
            if (0 == process_vector_no){
                LOG_WARN << "sleep 1 and do nothing!";
                sleep(1);
                continue;
            }
            LOG_INFO << "zygote wait begin";
            while (((pid = wait(&status)) == -1) && (errno == EINTR)) ;
            LOG_INFO << "zygote wait end";

            //分析进程退出原因
            exit_code_analyze(pid, status);
            if (-1 == pid){
                LOG_ERROR << "wait err. errno:[" << errno << "] error msg:<" << strerror(errno) << "<";
                continue;
            }

            it                                      = m_app_runinfo.map_pid_.find(pid);
            if(it == m_app_runinfo.map_pid_.end()){
                LOG_ERROR << "an unknown sub process exit!";
                continue;
            }else {
                pprocess_node                       = it->second;
                pid                                 = fork_subproc(child_argv[0], (char **)child_argv);
                LOG_INFO << "fork to exec subprocess name:[" << pprocess_node->name_get()
                        << "] path:<" << pprocess_node->file_path_get() << ">";
                //将此条目删除  已经无用
                m_app_runinfo.map_pid_.erase(it);
                //添加新条目到map中
                m_app_runinfo.map_pid_.insert(pair<pid_t, process_node*>(pid, pprocess_node));
            }
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
//	t_project_datum.shmem_.detach();
}



