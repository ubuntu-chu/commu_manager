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

void run_led_on(void)
{
    if (false == t_project_datum.project_config_.run_led_on()){
        LOG_WARN << "run led on failed\n";
    }
}

void run_led_off(void)
{
    if (false == t_project_datum.project_config_.run_led_off()){
        LOG_WARN << "run led off failed\n";
    }
}

void alarm_led_on(void)
{
    if (false == t_project_datum.project_config_.alarm_led_on()){
        LOG_WARN << "alarm led on failed\n";
    }
}

void alarm_led_off(void)
{
    if (false == t_project_datum.project_config_.alarm_led_off()){
        LOG_WARN << "alarm led off failed\n";
    }
}

struct process_stat   *process_stat_ptr_get(char index)
{
    return t_project_datum.pprocess_stat_ + index;
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
    g_logFile.reset(new muduo::LogFile(log_file_path, 2* 1000 * 1000));
    muduo::Logger::setOutput(outputFunc);
    muduo::Logger::setFlush(flushFunc);
#endif
    //获取进程状态结构体数组指针
    t_project_datum.pprocess_stat_          = reinterpret_cast<struct process_stat   *>(t_project_datum.shmem_.attach());
    LOG_INFO << "t_project_datum.pprocess_stat_ = " << reinterpret_cast<int *>(t_project_datum.pprocess_stat_);

	LOG_INFO << "parse project xml config file: " << config_file_path;
	if (xml_parse(config_file_path, &t_project_datum.project_config_)){
		LOG_SYSFATAL << "project xml config file parse failed!";
	}
    Logger::setLogLevel(static_cast<muduo::Logger::LogLevel>(t_project_datum.project_config_.log_lev_get()));
//    Logger::setLogLevel(muduo::Logger::INFO);

    run_led_off();
    alarm_led_off();

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
    const char *child_argv[100]            = {0};
    struct process_stat   *pprocess_stat   = NULL;
    bool            comm_stat;
    struct timespec ts                      = {1, 0};
    int             rt;

	m_app_runinfo.map_pid_.clear();
    while(m_app_runinfo.m_status == enum_APP_STATUS_RUN){
        if (0 == process_vector_no){
            LOG_ERROR << "project xml config file error! none process want to fork to exec";
        }
        //依据配置文件创建进程
        for (i = 0; i < process_vector_no; i++){
            pprocess_node                   = process_conf.process_node_get(i);

            if (false == pprocess_node->is_existed()){
                continue;
            }
            //初始化进程通讯状态
            pprocess_stat                   = process_stat_ptr_get(pprocess_node->index_get());
            pprocess_stat->comm_stat        = def_PROCESS_COMM_FAILED;

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
            rt                                  = ::nanosleep(&ts, NULL);
            if ((rt == -1) && (errno == EINTR)){

            }

            comm_stat                           = false;
            //统计子进程的通讯状态
            for (i = 0; i < process_vector_no; i++){
                pprocess_node                   = process_conf.process_node_get(i);

                if (false == pprocess_node->is_existed()){
                    continue;
                }
                pprocess_stat                   = process_stat_ptr_get(pprocess_node->index_get());
                LOG_INFO << "name:[" << pprocess_node->name_get()
                        << "] path:<" << pprocess_node->file_path_get() << "> comm_stat = "
                        << pprocess_stat->comm_stat;
                if (pprocess_stat->comm_stat  == def_PROCESS_COMM_OK){
                    comm_stat                   = true;
//                    break;
                }
            }
            if (true == comm_stat){
                run_led_on();
            }else {
                run_led_off();
            }

#if 0
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
#endif
        }

    }

    return 0;
}

void zygote::channels_power_off(void)
{
    project_config	*pproject_config        = &t_project_datum.project_config_;
	power_config    &power_conf	            = pproject_config->power_config_get();
	int             power_vector_no         = power_conf.power_vector_no_get();
	int             i;

    //关闭所有通道电源
    for (i = 0; i < power_vector_no; i++){
        power_conf.power_node_get(i)->power_off();
    }
}

void zygote::quit(void)
{
    m_app_runinfo.m_status  = enum_APP_STATUS_EXIT;
}

int main(int argc, char**argv)
{
	char    config_file_path[150];
	char    log_file_path[150]    = "which ";
	const char *pbase_name       = ::basename(argv[0]);
	char    *ptmp;
	FILE    *stream;
	zygote  *pzygote;

	argc 			= 2;

#if 0
	config_file_path 	= argv[1];
#endif
	if (argc != 2){
		LOG_SYSFATAL << "argc must = 2";
	}
	strcat(log_file_path, pbase_name);
	stream                   = popen(log_file_path, "r" );
	if (NULL == stream){
		LOG_SYSFATAL << "popen failed!";
	}
	memset(log_file_path, '\0', sizeof(log_file_path));
	fread(log_file_path, sizeof(char), sizeof(log_file_path),  stream);
	pclose(stream);

	strcpy(config_file_path, log_file_path);
	ptmp                      = strrchr(log_file_path, '/');
	strcpy(ptmp, "/../log/");
	strcat(log_file_path, pbase_name);

	ptmp                      = strrchr(config_file_path, '/');
	strcpy(ptmp, "/../config/config.xml");

	pzygote                   = zygote::GetInstance();
    pzygote->init(log_file_path, config_file_path);
    pzygote->run();

	LOG_INFO << "program exit";
	//删除共享内存
	t_project_datum.shmem_.detach();
}



