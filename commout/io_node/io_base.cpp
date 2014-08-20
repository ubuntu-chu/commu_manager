#include "io_base.h"
#include <channel.h>
#include <utils.h>

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

    m_nLastSend                 += len;
    len_                        = len;
    snprintf(log_buf, sizeof(log_buf), "io-name[%s], io-func[%s]",
            pio_node_->name_get(), "io_base::send_package");
    utils::log_binary_buf(log_buf, pdata, len);

    //调用send_data来完成具体的发送数据工作
    return send_data(pdata, len);
}

//向通道写报文   虚函数  需要继承类进行重载
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
        //判断通道当前能否接收
        if (false == pchannel_->can_receive()){
            LOG_WARN << "drop data because half duplex in send";
            return false;
        }
        return pchannel_->on_read(pdata, len, flag);
    }
    return false;
}

void io_base::send_status_end(int len)
{
    if (NULL != pchannel_){
        pchannel_->send_status_set(false);
    }
    //实际发送的数据个数 与 应该发送的数据个数 不相等
    if (len != len_){
        LOG_WARN << pio_node_->name_get() << " io_base::" << __func__
                << " send failed!; expected [" << len_ << "] actual ["
                << len << "]";
    }
}

//电源控制
bool io_base::power_ctrl(char value)
{
    bool rt;

    //只有IO_TYPE_EXT_COM类型的io_node 才可进行电源控制
    if (IO_TYPE_EXT_COM != pio_node_->io_type_get()){
        return false;
    }
    io_com_ext_node *pio_com_ext_node =
            reinterpret_cast<io_com_ext_node *>(const_cast<io_node *>(pio_node_));
    rt                  = pio_com_ext_node->power_ctrl(value);
    if (false == rt){
        const char *msg[]     = {" power_off", " power_on"};

        //电源控制失败， 记录下失败信息
        LOG_WARN << pio_node_->name_get() << " io_base::" << __func__ << msg[value - '0'] << " failed!";
    }

    return rt;
}
