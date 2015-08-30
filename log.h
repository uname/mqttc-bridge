#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>

#define MAX_LOG_MSG_SIZE  1024

typedef enum {
    LOG_VERBOSE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;


static int logLevel = LOG_DEBUG;

void mqttLog(LogLevel level, const char *file, const int line, char *format, ...);

#define LOG_SET_LEVEL(level) do { logLevel = level; }while(0)
#define LOG_V(fmt, ...) mqttLog(LOG_VERBOSE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...) mqttLog(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) mqttLog(LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) mqttLog(LOG_WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) mqttLog(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_F(fmt, ...) mqttLog(LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
