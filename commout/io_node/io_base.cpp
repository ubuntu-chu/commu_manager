#include "io_base.h"

//初始化通信介质
bool io_base::init(void)
{
    LOG_DEBUG;
    return true;
}

//反初始化
bool io_base::uninit()
{
    LOG_DEBUG;
    return true;
}

//连接通信介质
bool io_base::connect(bool brelink)
{
    LOG_DEBUG;
    return true;
}

//断开通信介质，
bool io_base::disconnect(void)
{
    LOG_DEBUG;
    return true;
}

//报文拆帧后发送
int io_base::send_package(char *pdata, size_t len)
{
    m_nLastSend += len;

    return send_data(pdata, len);
}

//向通道写报文
int io_base::send_data(char *pdata, size_t len)
{
    LOG_DEBUG;
    return 0;
}

//从通道读报文
bool io_base::on_read(const char *pdata, size_t len, int flag)
{
    return true;
}



