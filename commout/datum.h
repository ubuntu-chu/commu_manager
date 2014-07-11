#ifndef    _DATUM_H
#define    _DATUM_H

#include <includes/config.h>
#include <includes/shmem.h>
#include <muduo/base/Singleton.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;


class project_datum{
public:
    project_datum(){}
    ~project_datum(){}


//private:
    class shmem		    shmem_;
    project_config      *pproject_config_;
    EventLoop           *pevent_loop_;

};

extern class project_datum  t_project_datum;













#endif




