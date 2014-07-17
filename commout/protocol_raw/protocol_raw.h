#ifndef _PROTOCOL_RAW_H
#define _PROTOCOL_RAW_H

#include <includes/includes.h>
#include <protocol.h>


class protocol_raw:public protocol{
public:
    protocol_raw():protocol("raw"){}
    virtual ~protocol_raw(){}

    //初始化规约
    virtual bool init();
    //反初始化
    virtual void uninit();


    virtual bool handle_timer(void);
//private:

};















#endif

