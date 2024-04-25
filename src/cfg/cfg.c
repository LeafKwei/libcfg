#include "cfg/cfg.h"

const char* cfg_errstr[] = 
{
    "Success",
    "Out of memery",
    "NULL pointer",
    "Out of bounds"
};

const char* cfg_strerror(cfg_errno errno)
{
    if(errno >= 0 && errno < (sizeof(cfg_errstr) / sizeof(char*)))     //注意，对数组使用sizeof时，得到的是数组内部元素大小之和
        return cfg_errstr[(int)errno];
    return "";
}