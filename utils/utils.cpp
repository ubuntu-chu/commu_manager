#include "utils.h"
#include <includes/includes.h>

namespace utils{

void log_binary_buf(const char *pmsg, const char *pbuf, int len)
{

    char print_buf[1000];
    int i;
    unsigned int buf_len = 0;

    for (i = 0; i < len; i++){
        buf_len += snprintf(&print_buf[buf_len], sizeof(print_buf)-buf_len,
                " %02x", static_cast<uint8>(pbuf[i]));
    }

    if (NULL == pmsg){
        LOG_INFO << "binary buf :(" << print_buf << ") " << len << " bytes";
    }else {
        LOG_INFO << pmsg << "; binary buf :(" << print_buf << ") [" << len << "] bytes";
    }
}

void print_err_msg(const char *pmsg, const char *pfile, int line)
{
    fprintf(stderr, "%s - %s:%d\n", pmsg, pfile, line);
}

void signal_handler_install(int signum, void (*handler)(int))
{
    struct sigaction act;

    sigemptyset (&act.sa_mask);
    act.sa_handler                  = handler;
    sigaction (signum, &act, NULL);
}



}

