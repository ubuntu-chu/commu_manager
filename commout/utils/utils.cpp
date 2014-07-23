#include "utils.h"
#include <includes/includes.h>

namespace utils{

void log_binary_buf(const char *pbuf, int len)
{

    char print_buf[1000];
    unsigned int i;

    for (i = 0; i < len; i++){
        len += snprintf(&print_buf[len], sizeof(print_buf)-len,
                " %02x", static_cast<uint8>(pbuf[i]));
    }


    LOG_INFO << "binary buf :(" << print_buf << ") " << len << " bytes";
}


}

