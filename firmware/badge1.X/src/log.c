#include "hw.h"
#include "log.h"


#if UART3_LOG_ENABLED

void log_str(const char* s) {
    while (*s) {
        tx_write(*(s++));
    }
    tx_write('\r');
    tx_write('\n');
}

#endif
