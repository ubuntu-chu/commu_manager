#include "io_base.h"
#include <channel.h>

//初始化通信介质
bool io_base::init(void)
{
    LOG_TRACE;
    return true;
}

//反初始化
bool io_base::uninit()
{
    LOG_TRACE;
    return true;
}

//连接通信介质
bool io_base::connect(bool brelink)
{
    LOG_TRACE;
    return true;
}

//断开通信介质，
bool io_base::disconnect(void)
{
    LOG_TRACE;
    return true;
}

//报文拆帧后发送
int io_base::send_package(char *pdata, size_t len)
{
    char log_buf[200];

    m_nLastSend += len;
    snprintf(log_buf, sizeof(log_buf), "io-name[%s], io-func[%s]",
            pio_node_->name_get(), "io_base::send_package");
    utils::log_binary_buf(log_buf, pdata, len);

    return send_data(pdata, len);
}

//向通道写报文
int io_base::send_data(char *pdata, size_t len)
{
    return 0;
}

//从通道读报文
bool io_base::on_read(const char *pdata, int len, int flag)
{
    char log_buf[200];

    snprintf(log_buf, sizeof(log_buf), "io-name[%s], io-func[%s]",
            pio_node_->name_get(), "io_base::on_read");
    utils::log_binary_buf(log_buf, pdata, len);
    if (NULL != pchannel_){
        if ((enum_WORK_TYPE_HALF_DUPLEX == pchannel_->duplextype_get())
                && (true == pchannel_->send_status_get())){
            LOG_ERROR << "drop data because half duplex in send";
            return false;
        }
        return pchannel_->on_read(pdata, len, flag);
    }
    return false;
}

void io_base::send_status_end(void)
{
    if (NULL != pchannel_){
        return pchannel_->send_status_set(false);
    }
}


