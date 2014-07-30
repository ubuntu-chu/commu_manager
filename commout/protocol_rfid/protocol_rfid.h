#ifndef _PROTOCOL_RFID_H
#define _PROTOCOL_RFID_H

#include <includes/includes.h>
#include <protocol.h>

#define     def_PROTOCOL_RFID_NAME          ("rfid")

class protocol_rfid:public protocol{
public:
    protocol_rfid():protocol(def_PROTOCOL_RFID_NAME){}
    virtual ~protocol_rfid(){}

    //初始化规约
    virtual bool init();
    //反初始化
    virtual void uninit();

    virtual int package_aframe(char* pdata, int len);
    virtual bool process_aframe(const char * pdata, int len, int iflag = 0);

    virtual int  validate_aframe(struct validate_aframe_info *pinfo, int& ipacklen);

    virtual bool handle_timer(void);
//private:

};















#endif

