#include "utils.h"
#include <includes/includes.h>

namespace utils{

void format_binary_buf(char *pdst_buf, int dst_buf_len, const char *pbuf, int len)
{
    int i;
    unsigned int buf_len = 0;

    for (i = 0; i < len; i++){
        buf_len += snprintf(&pdst_buf[buf_len], dst_buf_len - buf_len,
                " %02x", static_cast<uint8>(pbuf[i]));
    }
}

void log_binary_buf(const char *pmsg, const char *pbuf, int len)
{
    char print_buf[1000];

    format_binary_buf(print_buf, sizeof(print_buf), pbuf, len);
    if (NULL == pmsg){
        LOG_INFO << "binary buf :(" << print_buf << ") " << len << " bytes";
    }else {
        LOG_INFO << pmsg << "; binary buf :(" << print_buf << ") [" << len << "] bytes";
    }
}

void log_binary_buf_trace(const char *pmsg, const char *pbuf, int len)
{
    char print_buf[1000];

    format_binary_buf(print_buf, sizeof(print_buf), pbuf, len);
    if (NULL == pmsg){
        LOG_TRACE << "binary buf :(" << print_buf << ") " << len << " bytes";
    }else {
        LOG_TRACE << pmsg << "; binary buf :(" << print_buf << ") [" << len << "] bytes";
    }
}

void print_err_msg(const char *pmsg, const char *pfile, int line)
{
    fprintf(stderr, "%s - %s:%d\n", pmsg, pfile, line);
    fprintf(stdout, "%s - %s:%d\n", pmsg, pfile, line);
}

void print_errno_msg(const char *pmsg)
{
    int  errno_cpy                      = errno;
    char buf[100];

    if (NULL != pmsg){
        fprintf(stdout, "%s - errno[%d]:errmsg<%s>\n", pmsg, errno_cpy,
                strerror_r(errno_cpy, buf, sizeof(buf)-1));
    }else {
        fprintf(stdout, "errno[%d]:errmsg<%s>\n", errno_cpy,
                strerror_r(errno_cpy, buf, sizeof(buf)-1));
    }
}

void signal_handler_install(int signum, void (*handler)(int))
{
    struct sigaction act;

    sigemptyset (&act.sa_mask);
    act.sa_handler                  = handler;
    sigaction (signum, &act, NULL);
}



}

