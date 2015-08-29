#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG

#ifndef DEBUG
#define DEBUG_PRINT(fmt, ...)
#else
#define DEBUG_PRINT(fmt, ...)    do { \
        fprintf(stderr, "[%8s:%3d]\t", __FUNCTION__, __LINE__); \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
    }while(0)
#endif

#endif