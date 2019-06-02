#ifndef _LOG_H
#define	_LOG_H

// you can save some memory and ticks by setting this to 0
#define UART3_LOG_ENABLED 0

#if UART3_LOG_ENABLED
void log_str(const char* s);
void log_strln(const char* s);
#else
#define log_str(s)
#define log_strln(s)
#endif


#endif	/* _LOG_H */

