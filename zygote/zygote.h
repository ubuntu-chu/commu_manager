#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <includes/includes.h>

enum {
    enum_APP_STATUS_INIT = 0,
    enum_APP_STATUS_INIT_ERR,
    enum_APP_STATUS_RUN_NO_SUBPROC,
    enum_APP_STATUS_RUN,
    enum_APP_STATUS_EXIT,
};

struct _app_runinfo_{
    volatile sig_atomic_t              m_status;
    int                                 timer_fd_;

    map<pid_t, process_node *>          map_pid_;
    const char *                       config_file_path_;

    vector<char>                       subprocess_comm_stat_;
    bool                               should_log_;

};
typedef struct _app_runinfo_ app_runinfo_t;

class process_node;

class zygote{
public:
    zygote(){};
    ~zygote(){};

    static zygote *GetInstance(void);
    portBASE_TYPE run(void);
    portBASE_TYPE init(const char *log_file_path, const char *config_file_path);
    void quit(void);
    void exit_code_analyze(pid_t pid, int status);

    void channels_power_off(void);
    void comm_status_statistics(void);
    void sig_chld_handle(void);

    int  fork_subproc_from_config(void);

    pid_t fork_subproc(process_node *pprocess_node);
    pid_t _fork_subproc(const char *path, char *const argv[]);
private:
    zygote(const zygote &other);
    zygote &operator =(const zygote &other);

    static zygote     *m_pzygote;
    struct _app_runinfo_    m_app_runinfo;
};

#endif
