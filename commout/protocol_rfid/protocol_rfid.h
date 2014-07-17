#ifndef _PROTOCOL_RFID_H
#define _PROTOCOL_RFID_H

#include <includes/includes.h>
#include <protocol.h>


class protocol_rfid:public protocol{
public:
    protocol_rfid():protocol("rfid"){}
    virtual ~protocol_rfid(){}

    //初始化规约
    virtual bool init();
    //反初始化
    virtual void uninit();

    virtual bool handle_timer(void);
//private:

};















#endif

