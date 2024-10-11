#ifndef INI_H
#define INI_H

/**
 # @author: Leaf Kwei
 # @date: 2024/04/16
 # ini模块提供了ini配置文件的解析功能
 */

#include "cfg/cfg.h"
#define INI_LIMIT_LINE 1024            //配置文件的单行文本最大只能有1023字节

typedef enum ini_propname
{
    INI_PROP_LINECNT = 0               //行计数
} ini_propname;

typedef struct ini_prop
{
    int linecnt;                       //已读取的行计数
} ini_prop;

typedef struct ini_kv
{
    char* key;
    char* val;
} ini_kv;

typedef struct ini_pair
{
    char* key;
    char* val;
    struct ini_pair* prevpair;
    struct ini_pair* nextpair;
} ini_pair;

typedef struct ini_sect
{
    char* name;
    struct ini_pair* cursor;
    struct ini_pair* pairs;
    struct ini_sect* prevsect;
    struct ini_sect* nextsect;
} ini_sect;

typedef struct ini
{
    struct ini_prop prop;              //ini属性信息
    struct ini_sect* cursor;           //迭代时的sect指针
    struct ini_sect* sections;         //sect列表头指针
    struct ini_sect* sectcache;        //针对getValue函数的sect缓存
    int index;                         //缓冲区中有效文本的长度，同时也是下一个字符的存放位置
    char linebuffer[INI_LIMIT_LINE];   //文本行缓冲区
    int mapsize;                       //bit位图大小
    char* bitmap;                      //bit位图
} ini;

//--------------basic start--------------
/**
 * @brief 开始ini的解析，对ini对象进行初始化
 * @param iptr ini对象指针
 * @param bitsize bit位图的大小，bit位图可以加快对sect的搜索速度，可以根据配置文件中sect的数量决定位图大小
 * @return 初始化成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_start(struct ini* iptr, unsigned int bitsize);

/**
 * @brief 从一段文本中解析出ini配置信息，文本可以是多行的。此函数支持片段式的解析，详情见sample/ini/test_frag.c
 * @param iptr ini对象指针，解析得到的sect会保存到其中
 * @param str 文本段
 * @return 解析成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_parse(struct ini* iptr, const char* str);

/**
 * @brief 将ini对象的行缓冲区中的内容主动添加换行符并交由ini_parse处理，适用于最末尾的，不存在换行符的文本，等同于ini_parse(iptr, "\n")
 * @param iptr ini对象
 * @return 成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_flush(struct ini* iptr);

/**
 * @brief 获取给定的name(section)与key(pair)对应的value
 * @param iptr ini对象
 * @param name section的名称
 * @param key section下的pair的名称
 * @param r_value 保存value指针的指针，如果不为NULL，则对应的pair中的value指针将保存到其中
 * @return 当存在对应的value时，返回1，否则返回0(此时*r_value将会被设置为NULL)
 */
CFG_BOOL ini_getValue(struct ini* iptr, const char* name, const char* key, char** r_value);

/**
 * @biref 释放ini对象中的所有资源
 * @param iptr ini对象指针
 * @return 释放成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_end(struct ini* iptr);

//--------------basic end--------------

//--------------iterate start------------
/**
 * @brief 开始section遍历，调用ini_nextSect前需要首先调用此函数
 * @param iptr ini对象
 * @return 成功开启时返回0，失败时返回错误码
 */
CFG_ERRNO ini_beginSects(struct ini* iptr);

/**
 * @brief 获取下一个section
 * @param iptr ini对象
 * @param r_sect 保存section指针的指针，如果不为NULL，则将下一个section的指针保存到其中
 * @param r_name 保存section名称指针的指针，如果不为NULL，则将下一个section名称的指针保存到其中
 * @return 成功获取到section时返回1，遍历完毕时返回0
 */
CFG_BOOL ini_nextSect(struct ini* iptr, struct ini_sect** r_sect, char** r_name);

/**
 * @brief 开始pair遍历，调用ini_nextPair前需要首先调用此函数
 * @param sect 由ini_nextSect获取的section对象
 * @return 成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_beginPairs(struct ini_sect* sect);

/**
 * @brief 获取下一个pair
 * @param sect 由ini_nextSection获取的section对象
 * @param kv 用于保存pair中key和value的结构体，如果不为NULL，则key和value的指针将保存到其中
 * @return 成功获取到key和value时返回1，遍历完毕时返回0
 */
CFG_BOOL ini_nextPair(struct ini_sect* sect, struct ini_kv* kv);

//--------------iterate end--------------

//--------------expands start--------------
/**
 * @brief 获取指定名称的ini属性值
 * @param iptr ini对象
 * @param name 属性名称
 * @param mem 保存属性值的内存地址
 * @return 成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_getProperty(struct ini* iptr, ini_propname name, void* mem);

/**
 * @brief 获取指定名称的section
 * @param iptr ini对象
 * @param name section名称
 * @param r_sect 保存section指针的指针，如果不为NULL，则将section的指针保存到其中
 * @return  当存在对应的section时，返回1，否则返回0(此时*r_sect将会被设置为NULL)
 */
CFG_BOOL ini_getSection(struct ini* iptr, const char* name, struct ini_sect** r_sect);

/**
 * @brief 从指定的section中获取key所对应的value
 * @param sect section对象
 * @param key section下的pair的名称
 * @param r_value 保存value指针的指针，如果不为NULL，则对应的pair中的value指针将保存到其中
 * @return 当存在对应的value时，返回1，否则返回0(此时*r_value将会被设置为NULL)
 */
CFG_BOOL ini_getValueFrom(struct ini_sect* sect, const char* key, char** r_value);

//--------------expands end--------------

//--------------ini write start--------------
/**
 * @brief 添加一个键值对到ini对象中，当name指定的section不存在时会新建section，当存在key对应的键值对时，覆盖已有的键值对
 * @param iptr ini对象
 * @param name section名称
 * @param key 键
 * @param value 值
 * @return 成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_putPair(struct ini* iptr, const char* name, const char* key, const char* value);

/**
 * @brief 添加一个键值对到ini对象中，当name指定的section不存在或键值对已存在时不会添加
 * @param iptr ini对象
 * @param name section名称
 * @param key 键
 * @param value 值
 * @return 成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_addPair(struct ini* iptr, const char* name, const char* key, const char* value);

/**
 * @brief 从ini对象中删除一个键值对
 * @param iptr ini对象
 * @param name section名称
 * @param key 键
 * @return 成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_rmvPair(struct ini* iptr, const char* name, const char* key);

/**
 * @brief 向ini对象中添加一个section
 * @param iptr ini对象
 * @param name section名称
 * @return 成功时返回0，失败时返回错误码 
 */
CFG_ERRNO ini_addSection(struct ini* iptr, const char* name);

/**
 * @brief 清空一个section中的内容
 * @param iptr ini对象
 * @param name section名称
 * @return 成功时返回0，失败时返回错误码 
 */
CFG_ERRNO ini_clrSection(struct ini* iptr, const char* name);

/**
 * @brief 从ini对象中删除一个section
 * @param iptr ini对象
 * @param name section名称
 * @return 成功时返回0，失败时返回错误码 
 */
CFG_ERRNO ini_rmvSection(struct ini* iptr, const char* name);

/**
 * @brief 将ini对象中的内容转换为一个字符串
 * @param iptr ini对象
 * @param r_buf 保存字符串地址的指针，字符串将会保存在动态分配的内存中，这段内存应该由此函数的调用者释放
 * @param commentor 用于生成注释的函数，可为每个section生成一个注释
 * @return 成功时返回0，失败时返回错误码
 */
CFG_ERRNO ini_toString(struct ini* iptr, char** r_buf, char* (*commentor)(const char* name));

//--------------ini write end--------------

#endif