#include "../include/utils.h"


uint32_t get_curr_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}