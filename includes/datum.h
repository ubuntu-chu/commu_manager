#ifndef    _DATUM_H
#define    _DATUM_H

#include "project_param.h"
#include "config.h"
#include "shmem.h"
#include <muduo/base/Singleton.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

struct process_stat{

    char                       comm_stat;
};

class project_datum{
public:
    project_datum(){}
    ~project_datum(){}

    int heart_beat_interval_get(void)
    {
        return def_HEART_BEAT_INTERVAL_S;
    }


//private:
    class shmem		    shmem_;
//    project_config      *pproject_config_;
    project_config      project_config_;
    EventLoop           *pevent_loop_;

    struct process_stat   *pprocess_stat_;
};

extern class project_datum  t_project_datum;













#endif




