#include "protocol_rfid.h"


//初始化规约
bool protocol_rfid::init()
{

    return true;
}

//反初始化
void protocol_rfid::uninit()
{

}


bool protocol_rfid::handle_timer(void)
{
    LOG_DEBUG;
    return true;
}

