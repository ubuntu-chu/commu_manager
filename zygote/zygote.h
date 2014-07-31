#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <includes/includes.h>

enum {
    enum_APP_STATUS_INIT = 0,
    enum_APP_STATUS_RUN,
    enum_APP_STATUS_EXIT,
};

struct _app_runinfo_{
    volatile sig_atomic_t              m_status;
    int                                 timer_fd_;

    map<pid_t, process_node *>          map_pid_;
    const char *                       config_file_path_;


};
typedef struct _app_runinfo_ app_runinfo_t;

class zygote{
public:
    zygote(){};
    ~zygote(){};

    static zygote *GetInstance(void);
    portBASE_TYPE run(void);
    portBASE_TYPE init(const char *log_file_path, const char *config_file_path);
    void quit(void);
    void exit_code_analyze(pid_t pid, int status);

    pid_t fork_subproc(const char *path, char *const argv[]);
private:
    zygote(const zygote &other);
    zygote &operator =(const zygote &other);

    static zygote     *m_pzygote;
    struct _app_runinfo_    m_app_runinfo;
};

#endif
