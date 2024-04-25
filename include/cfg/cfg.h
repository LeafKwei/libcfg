/*
# @author: Leaf Kwei
# @date: 2024/04/15
# CFG模块定义了库文件版本，并提供了通用的错误码和错误信息获取函数，错误信息定义在了cfg.c中
*/

#define CFG_VERSION "1.0.0"
#define CFG_ERRNO cfg_errno
#define CFG_BOOL cfg_bool

typedef enum
{
    CFG_BOOL_FLASE = 0,
    CFG_BOOL_TRUE = 1
} cfg_bool;

typedef enum
{
    CFG_ERR_NONE = 0,     //无错误      
    CFG_ERR_OOM,          //内存溢出
    CFG_ERR_NULLPTR,      //空指针
    CFG_ERR_OOB           //超出范围
} cfg_errno;

extern const char* cfg_strerror(cfg_errno errno);