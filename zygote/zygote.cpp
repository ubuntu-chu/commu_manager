#include <datum.h>
#include "zygote.h"
#include <parse.h>
#include <utils.h>
#include <sys/wait.h>

//def_DBG_IN_PC_SINGLE宏 当在pc端调试时，此宏的值定义为1  嵌入式版本时 定义为0
#define     def_DBG_IN_PC_SINGLE        (0)

#if (def_DBG_IN_PC_SINGLE > 0)
#define     PROCESS_PREFIX_PATH         "/home/barnard/work/commu_manager/manager/"
#endif

using std::string;
using std::pair;

boost::scoped_ptr<muduo::LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
    //追加到日志文件后面
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

//段错误处理函数
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

//系统初始化函数
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

    //获取进程状态结构体数组指针 创建进程间共享内存
    if (t_project_datum.shmem_.create()){
        m_app_runinfo.m_status                  = enum_APP_STATUS_INIT_ERR;
		return -1;
    }
    //将共享内存映射到本进程的地址空间中
    t_project_datum.pprocess_stat_          = reinterpret_cast<struct process_stat   *>(t_project_datum.shmem_.attach());
    LOG_INFO << "t_project_datum.pprocess_stat_ = " << reinterpret_cast<int *>(t_project_datum.pprocess_stat_);

	LOG_INFO << "parse project xml config file: " << config_file_path;
	//解析配置文件
	if (xml_parse(config_file_path, &t_project_datum.project_config_)){
        m_app_runinfo.m_status                  = enum_APP_STATUS_INIT_ERR;
		LOG_ERROR << "project xml config file parse failed!";

		return -1;
	}
	//设置日志级别
    Logger::setLogLevel(static_cast<muduo::Logger::LogLevel>(t_project_datum.project_config_.log_lev_get()));
    LOG_INFO << "subprocess heartbeat sec = " << t_project_datum.project_config_.heartbeat_s_get();

    run_led_off();
    alarm_led_off();

    return 0;
}

//进程退出原因分析
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

//创建子进程执行程序
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
            //exec函数族执行出错
            LOG_ERROR << "execv rt:[" << rt << "] errno:["<< errno
                    << "] error msg:<" << strerror(errno) << ">";
            utils::print_errno_msg("zygote::_fork_subproc->execvp");
        }
        exit(1);
    }

    //返回子进程pid
    return pid;
}

pid_t zygote::fork_subproc(process_node *pprocess_node)
{
    const char *child_argv[5]          = {0};
    struct process_stat   *pprocess_stat   = NULL;
    int         index                   = pprocess_node->index_get();

    //初始化进程通讯状态为失败状态
    pprocess_stat                       = process_stat_ptr_get(index);
    pprocess_stat->comm_stat            = def_PROCESS_COMM_FAILED;
    m_app_runinfo.subprocess_comm_stat_[index]  = def_PROCESS_COMM_FAILED;
    m_app_runinfo.should_log_           = true;
    child_argv[0]                       = pprocess_node->file_path_get();
    child_argv[1]                       = m_app_runinfo.config_file_path_;
    return _fork_subproc(child_argv[0], (char **)child_argv);
}

//统计子进程的通讯状态 并将状态信息记录到log文件中
void zygote::comm_status_statistics(void)
{
    bool            comm_stat               = false;
    bool            should_log              = false;
	project_config	*pproject_config        = &t_project_datum.project_config_;
	process_config  &process_conf	        = pproject_config->process_config_get();
	process_node    *pprocess_node;
	//子进程容器数量
	int             process_vector_no       = process_conf.process_vector_no_get();
    struct process_stat   *pprocess_stat   = NULL;
	int             i, index;

    //统计子进程的通讯状态 循环处理所有子进程
    for (i = 0; i < process_vector_no; i++){
        pprocess_node                       = process_conf.process_node_get(i);

        //判断子进程的 exist属性值
        if (false == pprocess_node->is_existed()){
            continue;
        }
        index                                      = pprocess_node->index_get();
        //获取子进程的状态结构体指针   状态结构体是一个数组
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
    //m_app_runinfo.should_log_ 在进程启动时 被赋值为true
    if (m_app_runinfo.should_log_ == true){
        m_app_runinfo.should_log_                  = false;
        should_log                                 = true;
    }
    if (should_log == true){
        //记录所有子进程状态信息到日志文件中
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

//处理子进程退出  子进程推出后 需要重启此子进程
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
        //没找到退出的子进程pid
        if(it == m_app_runinfo.map_pid_.end()){
            LOG_ERROR << "an unknown sub process exit!";
            continue;
        }else {
            pprocess_node                       = it->second;
            LOG_INFO << "fork to exec subprocess name:[" << pprocess_node->name_get()
                    << "] path:<" << pprocess_node->file_path_get() << ">";
            //重启退出的子进程
            pid                                 = fork_subproc(pprocess_node);
            //将此条目删除  已经无用
            m_app_runinfo.map_pid_.erase(it);
            //添加新条目到map中
            m_app_runinfo.map_pid_.insert(pair<pid_t, process_node*>(pid, pprocess_node));
        }
    }
    if (pid < 0){
        utils::print_errno_msg("waitpid");
    }
}

//从配置文件里创建子进程
int  zygote::fork_subproc_from_config(void)
{
	project_config	*pproject_config            = &t_project_datum.project_config_;
	process_config  &process_conf	            = pproject_config->process_config_get();
	process_node    *pprocess_node;
	int             process_vector_no           = process_conf.process_vector_no_get();
    int             i, index;
    pid_t           pid;

	m_app_runinfo.map_pid_.clear();
	//没有子进程需要创建
    if (0 == process_vector_no){
        LOG_ERROR << "project xml config file error! none process want to fork to exec";
        return -1;
    }else {
        //创建子进程通讯状态数组   需要创建的子进程个数为process_vector_no
        m_app_runinfo.subprocess_comm_stat_.reserve(process_vector_no);
        //依据配置文件创建进程
        for (i = 0; i < process_vector_no; i++){
            pprocess_node                       = process_conf.process_node_get(i);
            index                               = pprocess_node->index_get();

            //要对配置的合法性进行判断
            //进程索引不对  则停止创建过程  一定不要改动配置文件中的进程索引
            if (index != i){
                //杀死已经创建的进程
                g_sig_quit_                     = 1;
                LOG_ERROR << "process[" << pprocess_node->name_get()
                        << "] index(" << index << ") != "<< i;
                return -1;
            //进程索引范围不正确
            }else if (index > process_vector_no){
                //杀死已经创建的进程
                g_sig_quit_                     = 1;
                LOG_ERROR << "process[" << pprocess_node->name_get()
                        << "] index(" << index << ") > process numb(" << process_vector_no << ")";
                return -1;
            }
            //子进程通讯状态 初始化默认值
            m_app_runinfo.subprocess_comm_stat_[index]
                                                = def_PROCESS_COMM_FAILED;
            //判断进程是否存在  即配置文件中的exist属性值
            if (false == pprocess_node->is_existed()){
                continue;
            }
            //创建子进程运行
            pid                                 = fork_subproc(pprocess_node);

            LOG_INFO << "fork to exec subprocess name:[" << pprocess_node->name_get()
                    << "] path:<" << pprocess_node->file_path_get() << ">";
            //将子进程pid和相应的process_node* 插入到map_pid_映射中
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
        case enum_APP_STATUS_INIT_ERR:

            //初始化错误  杀死自己
            ::raise (SIGKILL);
            break;

        case enum_APP_STATUS_INIT:

            if (0 == fork_subproc_from_config()){
                //创建子进程成功 系统进入enum_APP_STATUS_RUN状态
                m_app_runinfo.m_status       = enum_APP_STATUS_RUN;
            }else{
                //创建子进程失败
                m_app_runinfo.m_status       = enum_APP_STATUS_RUN_NO_SUBPROC;
            }
            break;

        case enum_APP_STATUS_RUN_NO_SUBPROC:

            LOG_WARN << "no subprocess exsit; sleep 1 and do nothing!";
            ::nanosleep(&ts, NULL);
            break;

        case enum_APP_STATUS_RUN:
        {
            //睡眠1s
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
        //处理本进程退出事件
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

//退出函数
void zygote::quit(void)
{
    map<pid_t, process_node *>::iterator it;
    pid_t           pid;

    //杀死本进程创建的所有子进程
	for (it = m_app_runinfo.map_pid_.begin(); it != m_app_runinfo.map_pid_.end(); ++it){
	    pid                                 = it->first;
        LOG_INFO << "send SIGKILL to subprocess; pid = [" << pid << "]";
        //发送sigkill信号到子进程
	    ::kill(pid, SIGKILL);
	}
    m_app_runinfo.m_status                  = enum_APP_STATUS_EXIT;
    //关闭所有通道电源
    channels_power_off();
    //关闭run和alarm指示灯
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
	//获取进程位置的绝对路径  使用popen函数
	stream                                  = popen(log_file_path, "r" );
	if (NULL == stream){
		LOG_SYSFATAL << "popen failed!";
	}
	memset(log_file_path, '\0', sizeof(log_file_path));
	//从流中读取which命令的返回值到log_file_path中
	fread(log_file_path, sizeof(char), sizeof(log_file_path),  stream);
	//关闭流
	pclose(stream);

	//获取log文件的存放路径
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
	//获取config.xml文件绝对路径
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



