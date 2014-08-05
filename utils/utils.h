#ifndef  _UTILIS_H
#define  _UTILIS_H


namespace utils{


void log_binary_buf(const char *pmsg, const char *pbuf, int len);
void log_binary_buf_trace(const char *pmsg, const char *pbuf, int len);

void print_err_msg(const char *pmsg, const char *pfile, int line);

void signal_handler_install(int signum, void (*handler)(int));

void print_errno_msg(const char *pmsg);



}


#endif

