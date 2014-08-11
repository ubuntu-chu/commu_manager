#include <datum.h>
#include "zygote.h"
#include <parse.h>
#include <utils.h>
#include <sys/wait.h>

#define     def_DBG_IN_PC_SINGLE        (0)

#if (def_DBG_IN_PC_SINGLE > 0)
#define     PROCESS_PREFIX_PATH         "/home/barnard/work/commu_manager/manager/"
#endif

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

volatile sig_atomic_t              g_sig_quit_;
volatile sig_atomic_t              g_sig_chld_;
volatile sig_atomic_t              g_sig_;

void run_led_on(void)
{
    if (false == t_project_datum.project_config_.run_led_on()){
        LOG_WARN << "run led on failed";
    }
}

void run_led_off(void)
{
    if (false == t_project_datum.project_config_.run_led_off()){
        LOG_WARN << "run led off failed";
    }
}

void alarm_led_on(void)
{
    if (false == t_project_datum.project_config_.alarm_led_on()){
        LOG_WARN << "alarm led on failed";
    }
}

void alarm_led_off(void)
{
    if (false == t_project_datum.project_config_.alarm_led_off()){
        LOG_WARN << "alarm led off failed";
    }
}

struct process_stat   *process_stat_ptr_get(char index)
{
    return t_project_datum.pprocess_stat_ + index;
}

void sig_handler_quit(int signum, siginfo_t *info, void *ptr)
{
	g_sig_quit_                             = 1;
	g_sig_                                  = signum;
}

void sig_handler_chld(int signum, siginfo_t *info, void *ptr)
{
	g_sig_chld_                             = 1;
}

void sig_handler_segv(int signum, siginfo_t *info, void *ptr)
{
    /* 动态链接库的映射地址是动态的，需要将maps文件打印出来 */
    char file[64], buffer[1032];
    pid_t pid                                   = getpid();

    printf("signal[%d] catched\n", signum);
    LOG_ERROR << "signal[" << signum << "] catched";

    printf("\n-------------------------- process pid  --------------------------\n");

    printf("\n process pid = [%d]\n", pid);
    LOG_ERROR << "process pid = [" << pid << "]";

    printf("\n-------------------------- process MAPS --------------------------\n");
    LOG_ERROR << "-------------------------- process MAPS --------------------------";

    snprintf(file, sizeof(file), "/proc/%d/maps", pid);
    FILE *fp                                    = fopen(file, "r");
    if (NULL != fp){
        while(fgets(buffer, 1024, fp)){
            fputs(buffer, stdout);
            buffer[strlen(buffer) - 1]          = '\0';
            LOG_ERROR << buffer;
        }
    }else{
        printf("read pid MAPS failed!\n");
        LOG_ERROR << "read pid MAPS failed!";
    }

    printf("\n----------------------- process called frame -----------------------\n");
    LOG_ERROR << "----------------------- process called frame -----------------------";

    static int iTime = 0;
    if (iTime++ >= 1){ /* 容错处理：如果访问 ucontext_t 结构体时产生错误会进入该分支 */
        printf("%s reenter is not allowed!\n", __FUNCTION__);
        LOG_ERROR << __FUNCTION__ << "reenter is not allowed!";
        abort();
    }

    void * array[25];
    size_t size;
    char **strings;
    size_t i;

    size                                        = backtrace (array, sizeof(array)/sizeof(void *));
    strings                                     = backtrace_symbols (array, size);

    printf ("backtrace ( %zd frame deep):\n", size);
    LOG_ERROR << "backtrace (" << size << ") frame deep:";

    for (i = 0; i < size; i++){
        printf ("%d: %s\n", i, strings[i]);
        LOG_ERROR << i << ": " << strings[i];
    }
    free (strings);

#if 0
    if (NULL != ptr){
        ucontext_t* ptrUC = (ucontext_t*)ptr;
        int *pgregs                           = (int*)(&(ptrUC->uc_mcontext.gregs));
        int eip = pgregs[REG_EIP];
        if (eip != array[i]){ /* 有些处理器会将出错时的 EIP 也入栈 */
            printf("signal[%d] catched when running code at %x\n", signum, (char*)array[i] - 1);
        }
        printf("signal[%d] catched when running code at %x\n", signum, eip); /* 出错地址 */
    }else{
        printf("signal[%d] catched when running code at unknown address\n", signum);
    }
#endif

    printf("-------------------------------------------------------------------------\n");

    abort();
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
	g_sig_quit_                             = 0;
	g_sig_                                  = 0;
	g_sig_chld_                             = 0;

    //获取进程名字
    process_name_str                        = ProcessInfo::procname();

    //安装信号处理函数
    if ((utils::signal_handler_install(SIGTERM, sig_handler_quit))
            || (utils::signal_handler_install(SIGQUIT, sig_handler_quit))
            || (utils::signal_handler_install(SIGINT, sig_handler_quit))
            || (utils::signal_handler_install(SIGABRT, sig_handler_quit))
            || (utils::signal_handler_install(SIGCHLD, sig_handler_chld))
            || (utils::signal_handler_install(SIGSEGV, sig_handler_segv))){
        m_app_runinfo.m_status                  = enum_APP_STATUS_INIT_ERR;
		LOG_ERROR << "sig handler install failed!";

		return -1;
	}
    //设置日志文件名称
    g_logFile.reset(new muduo::LogFile(log_file_path, 10 * 1000 * 1000));
    muduo::Logger::setOutput(outputFunc);
    muduo::Logger::setFlush(flushFunc);
    //获取进程状态结构体数组指针
    if (t_project_datum.shmem_.create()){
        m_app_runinfo.m_status                  = enum_APP_STATUS_INIT_ERR;
		return -1;
    }
    t_project_datum.pprocess_stat_          = reinterpret_cast<struct process_stat   *>(t_project_datum.shmem_.attach());
    LOG_INFO << "t_project_datum.pprocess_stat_ = " << reinterpret_cast<int *>(t_project_datum.pprocess_stat_);

	LOG_INFO << "parse project xml config file: " << config_file_path;
	if (xml_parse(config_file_path, &t_project_datum.project_config_)){
        m_app_runinfo.m_status                  = enum_APP_STATUS_INIT_ERR;
		LOG_ERROR << "project xml config file parse failed!";

		return -1;
	}
    Logger::setLogLevel(static_cast<muduo::Logger::LogLevel>(t_project_datum.project_config_.log_lev_get()));
    LOG_INFO << "subprocess heartbeat sec = " << t_project_datum.project_config_.heartbeat_s_get();

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

pid_t zygote::_fork_subproc(const char *path, char *const argv[])
{
    pid_t pid;

    pid                             = fork();
    if (pid == -1) {
        LOG_WARN << "fork() err. errno:[" << errno << "] error msg:<" << strerror(errno) << ">";
        utils::print_errno_msg("zygote::_fork_subproc->fork");
        exit(1);
    }
    //子进程
    if (pid == 0) {
        int  rt;
#if (def_DBG_IN_PC_SINGLE > 0)
        char process_path[100]              = PROCESS_PREFIX_PATH;

        strcat(process_path, "bin/");
        strcat(process_path, path);
        rt                                  = execv(process_path, argv);
#else
        rt                                  = execvp(path, argv);
#endif
        if (rt < 0) {
            LOG_ERROR << "execv rt:[" << rt << "] errno:["<< errno
                    << "] error msg:<" << strerror(errno) << ">";
            utils::print_errno_msg("zygote::_fork_subproc->execvp");
        }
        exit(1);
    }

    return pid;
}

pid_t zygote::fork_subproc(process_node *pprocess_node)
{
    const char *child_argv[5]          = {0};
    struct process_stat   *pprocess_stat   = NULL;
    int         index                   = pprocess_node->index_get();

    //初始化进程通讯状态
    pprocess_stat                       = process_stat_ptr_get(index);
    pprocess_stat->comm_stat            = def_PROCESS_COMM_FAILED;
    m_app_runinfo.subprocess_comm_stat_[index]  = def_PROCESS_COMM_FAILED;
    m_app_runinfo.should_log_           = true;
    child_argv[0]                       = pprocess_node->file_path_get();
    child_argv[1]                       = m_app_runinfo.config_file_path_;
    return _fork_subproc(child_argv[0], (char **)child_argv);
}


void zygote::comm_status_statistics(void)
{
    bool            comm_stat               = false;
    bool            should_log              = false;
	project_config	*pproject_config        = &t_project_datum.project_config_;
	process_config  &process_conf	        = pproject_config->process_config_get();
	process_node    *pprocess_node;
	int             process_vector_no       = process_conf.process_vector_no_get();
    struct process_stat   *pprocess_stat   = NULL;
	int             i, index;

    //统计子进程的通讯状态
    for (i = 0; i < process_vector_no; i++){
        pprocess_node                       = process_conf.process_node_get(i);

        if (false == pprocess_node->is_existed()){
            continue;
        }
        index                                      = pprocess_node->index_get();
        pprocess_stat                              = process_stat_ptr_get(index);
        LOG_TRACE << "name:[" << pprocess_node->name_get()
                << "] path:<" << pprocess_node->file_path_get() << "> comm_stat = "
                << pprocess_stat->comm_stat;
        if (pprocess_stat->comm_stat  == def_PROCESS_COMM_OK){
            comm_stat                              = true;
//                    break;
        }else {
        }
        //判断子进程通讯状态有无改变
        if (m_app_runinfo.subprocess_comm_stat_[index] != pprocess_stat->comm_stat){
            m_app_runinfo.subprocess_comm_stat_[index] = pprocess_stat->comm_stat;
            should_log                             = true;
        }
    }
    if (m_app_runinfo.should_log_ == true){
        m_app_runinfo.should_log_                  = false;
        should_log                                 = true;
    }
    if (should_log == true){
        utils::log_binary_buf("subprocess comm stat array: ",
                &(*m_app_runinfo.subprocess_comm_stat_.begin()), process_vector_no);
    }
    if (true == comm_stat){
        run_led_on();
        alarm_led_off();
    }else {
        run_led_off();
        alarm_led_on();
    }
}

void zygote::sig_chld_handle(void)
{
    pid_t           pid;
	int             status;
    map<pid_t, process_node *>::iterator it;
	process_node    *pprocess_node;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0){
        //分析进程退出原因
        exit_code_analyze(pid, status);
        it                                      = m_app_runinfo.map_pid_.find(pid);
        if(it == m_app_runinfo.map_pid_.end()){
            LOG_ERROR << "an unknown sub process exit!";
            continue;
        }else {
            pprocess_node                       = it->second;
            LOG_INFO << "fork to exec subprocess name:[" << pprocess_node->name_get()
                    << "] path:<" << pprocess_node->file_path_get() << ">";
    #if 1
            pid                                 = fork_subproc(pprocess_node);
            //将此条目删除  已经无用
            m_app_runinfo.map_pid_.erase(it);
            //添加新条目到map中
            m_app_runinfo.map_pid_.insert(pair<pid_t, process_node*>(pid, pprocess_node));
    #endif
        }
    }
    if (pid < 0){
        utils::print_errno_msg("waitpid");
    }
}

int  zygote::fork_subproc_from_config(void)
{
	project_config	*pproject_config            = &t_project_datum.project_config_;
	process_config  &process_conf	            = pproject_config->process_config_get();
	process_node    *pprocess_node;
	int             process_vector_no           = process_conf.process_vector_no_get();
    int             i, index;
    pid_t           pid;

	m_app_runinfo.map_pid_.clear();
    if (0 == process_vector_no){
        LOG_ERROR << "project xml config file error! none process want to fork to exec";
        return -1;
    }else {
        //创建子进程通讯状态数组
        m_app_runinfo.subprocess_comm_stat_.reserve(process_vector_no);
        //依据配置文件创建进程
        for (i = 0; i < process_vector_no; i++){
            pprocess_node                       = process_conf.process_node_get(i);
            index                               = pprocess_node->index_get();

            //要对配置的合法性进行判断
            if (index != i){
                //杀死已经创建的进程
                g_sig_quit_                     = 1;
                LOG_ERROR << "process[" << pprocess_node->name_get()
                        << "] index(" << index << ") != "<< i;
                return -1;
            }else if (index > process_vector_no){
                //杀死已经创建的进程
                g_sig_quit_                     = 1;
                LOG_ERROR << "process[" << pprocess_node->name_get()
                        << "] index(" << index << ") > process numb(" << process_vector_no << ")";
                return -1;
            }
            //写入默认值
            m_app_runinfo.subprocess_comm_stat_[index]
                                                = def_PROCESS_COMM_FAILED;
            if (false == pprocess_node->is_existed()){
                continue;
            }
            pid                                 = fork_subproc(pprocess_node);

            LOG_INFO << "fork to exec subprocess name:[" << pprocess_node->name_get()
                    << "] path:<" << pprocess_node->file_path_get() << ">";
            m_app_runinfo.map_pid_.insert(pair<pid_t, process_node*>(pid, pprocess_node));
        }
    }

    return 0;
}

portBASE_TYPE zygote::run()
{
    //定时1s
    struct timespec ts                      = {1, 0};
    int             rt;

    while(m_app_runinfo.m_status != enum_APP_STATUS_EXIT){
        switch (m_app_runinfo.m_status){
        //初始化错误  进程不做任何事情 只是检查父进程是否退出
        case enum_APP_STATUS_INIT_ERR:

            ::raise (SIGKILL);
            break;

        case enum_APP_STATUS_INIT:

            if (0 == fork_subproc_from_config()){
                m_app_runinfo.m_status       = enum_APP_STATUS_RUN;
            }else{
                m_app_runinfo.m_status       = enum_APP_STATUS_RUN_NO_SUBPROC;
            }
            break;

        case enum_APP_STATUS_RUN_NO_SUBPROC:

            LOG_WARN << "no subprocess exsit; sleep 1 and do nothing!";
            ::nanosleep(&ts, NULL);
            break;

        case enum_APP_STATUS_RUN:
        {
            rt                                  = ::nanosleep(&ts, NULL);
            if ((rt == -1) && (errno == EINTR)){
                LOG_INFO << "a signal occured, process it";
            }
            //统计子进程的通讯状态
            comm_status_statistics();
        }
            break;
        default:
            break;
        }
        //处理子进程退出事件
        if (1 == g_sig_chld_){
            g_sig_chld_                     = 0;
            LOG_INFO << "SIGCHLD deliver, call sig_chld_handle() to process it";
            sig_chld_handle();
        }
        if (1 == g_sig_quit_){
            g_sig_quit_                     = 0;
            LOG_INFO << "SIG[" << g_sig_ << "] deliver, call quit() to process it";
            quit();
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
    map<pid_t, process_node *>::iterator it;
    pid_t           pid;

	for (it = m_app_runinfo.map_pid_.begin(); it != m_app_runinfo.map_pid_.end(); ++it){
	    pid                                 = it->first;
        LOG_INFO << "send SIGKILL to subprocess; pid = [" << pid << "]";
	    ::kill(pid, SIGKILL);
	}
    m_app_runinfo.m_status                  = enum_APP_STATUS_EXIT;
    channels_power_off();
    run_led_off();
    alarm_led_off();
}

int main(int argc, char**argv)
{
	char    config_file_path[150];
	char    log_file_path[150]             = "which ";
	const char *pbase_name                = ::basename(argv[0]);
	char    *ptmp;
	FILE    *stream;
	zygote  *pzygote;


#if (def_DBG_IN_PC_SINGLE > 0)
	strcpy(config_file_path, PROCESS_PREFIX_PATH);
	strcat(config_file_path, "config/config.xml");
	strcpy(log_file_path, PROCESS_PREFIX_PATH);
	strcat(log_file_path, "log/");
	strcat(log_file_path, pbase_name);
#else
	strcat(log_file_path, pbase_name);
	stream                                  = popen(log_file_path, "r" );
	if (NULL == stream){
		LOG_SYSFATAL << "popen failed!";
	}
	memset(log_file_path, '\0', sizeof(log_file_path));
	fread(log_file_path, sizeof(char), sizeof(log_file_path),  stream);
	pclose(stream);

	strcpy(config_file_path, log_file_path);
	ptmp                                    = strrchr(log_file_path, '/');
	strcpy(ptmp, "/../log/");
	//依次执行mkdir -p dump ; cp -r * dump/   将log目录下的所有log文件拷贝到dump文件夹中
	{
        char    system_cmd[400];

        strcpy(system_cmd, "/bin/mkdir -p ");
        strcat(system_cmd, log_file_path);
        strcat(system_cmd, "../log_dump/");
//        LOG_INFO << system_cmd;
        system(system_cmd);

        strcpy(system_cmd, "/bin/cp -r ");
        strcat(system_cmd, log_file_path);
        strcat(system_cmd, "* ");
        strcat(system_cmd, log_file_path);
        strcat(system_cmd, "../log_dump/");
//        LOG_INFO << system_cmd;
        system(system_cmd);
	}
	strcat(log_file_path, pbase_name);

	ptmp                                    = strrchr(config_file_path, '/');
	strcpy(ptmp, "/../config/config.xml");
#endif

	pzygote                                 = zygote::GetInstance();
    pzygote->init(log_file_path, config_file_path);
    pzygote->run();

	LOG_INFO << "program exit";
	//删除共享内存
	t_project_datum.shmem_.detach();
    t_project_datum.shmem_.destroy();
}



