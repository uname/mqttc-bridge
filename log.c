#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int currentTime(const char *file, const int line, char *currTime, size_t size)
{
	struct tm *ptm = NULL;
	time_t t;
    char szTime[256];
    
	t = time(NULL);
	ptm = localtime(&t);
	
	memset(szTime, 0, sizeof(szTime));
    
	sprintf(szTime, "[%d-%02d-%02d %02d:%02d:%02d][%s:%03d] ", (ptm->tm_year + 1900),
		     ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, file, line);
             
	strncpy(currTime, szTime, size);
    
	return 0;
}

void mqttLog(LogLevel level, const char *file, const int line, char *format, ...)
{
    va_list va;
    char buf[1024];
    char _time[512];
        
    if(level < logLevel) {
        return;
    }
    
    currentTime(file, line, _time, sizeof(_time));
    fprintf(stderr, "%s", _time);
    
    va_start(va, format);
    vsnprintf(buf, MAX_LOG_MSG_SIZE, format, va);
    fprintf(stderr, "%s", buf);
    
    va_end(va);
}
