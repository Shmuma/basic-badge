#ifndef _LOG_H
#define	_LOG_H

// you can save some memory and ticks by setting this to 0
#define UART3_LOG_ENABLED 1

#if UART3_LOG_ENABLED
void log_str(const char* s);
#else
#define log_str(s)
#endif


#endif	/* _LOG_H */

