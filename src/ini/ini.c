#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include "cfg/cfg.h"
#include "cfg/ini.h"

#define BITWISE (8)

#define NeverNULL(ptr) assert(ptr != NULL)
#define NOP ((void*) NULL)
#define CEILBYTE(size) (((size) / BITWISE) + ((size) % BITWISE != 0 ? 1 : 0))

//--------------------private--------------------
static CFG_ERRNO init_bitmap(struct ini* iptr, unsigned int bitsize)
{
    NeverNULL(iptr);
    if(bitsize == 0) goto disable_bitmap;

    int mapsize = CEILBYTE(bitsize);
    char* mem = (char*) calloc(mapsize, 1);
    if(mem == NULL) return CFG_ERR_OOM;

    iptr -> mapsize = mapsize;
    iptr -> bitmap = mem;

    return CFG_ERR_NONE; 

disable_bitmap:
    iptr -> bitmap = NULL;
    iptr -> mapsize = 0;
    return CFG_ERR_NONE;
}

static CFG_ERRNO copy_line(struct ini* iptr, const char* str, int* index)
{
    NeverNULL(iptr);
    NeverNULL(str);
    assert(*index >= 0 && *index < strlen(str));

    int lineapp = 0;          //行计数增量
    int str_index = *index;
    int empty_len = 0;        //行缓冲区还可保存多长文本
    int remain_len = 0;       //str剩余的文本长度
    int copy_len = 0;         //拷贝长度
    char* newline = NULL;

    empty_len = (INI_LIMIT_LINE - iptr -> index) - 1;     //保留末尾的'\0'，因此需要减1
    remain_len = strlen(str + str_index);
    newline = strchr(str + str_index, '\n');
    
    /* 不存在换行符时，拷贝剩余所有文本 */
    if(newline == NULL)
    {
        empty_len = empty_len - 1;      //需要为'\n'预留空间，因此不能将所有剩余空间用于保存文本
        copy_len = remain_len;
    }

    /* 存在换行符时，拷贝到换行符所在位置 */
    else
    {
        copy_len = ((int)(newline - (str + str_index))) + 1;
        lineapp += 1;
    }

    /* 检查行缓冲区剩余空间是否足够保存文本 */
    if(empty_len < copy_len)
    {
        return CFG_ERR_OOB;        //文本过长
    }

    memcpy(iptr -> linebuffer + iptr -> index, str + str_index, copy_len);
    iptr -> index += copy_len;
    iptr -> prop.linecnt += lineapp;
    str_index += copy_len;
    *index = str_index;
    return CFG_ERR_NONE;
}

static CFG_BOOL has_line(const char* buffer)
{
    NeverNULL(buffer);
    return strchr(buffer, '\n') != NULL ? CFG_BOOL_TRUE : CFG_BOOL_FLASE;
}

/* ! 此函数会将文本末尾的换行符消除 */
static void filter_comments(struct ini* iptr)
{
    NeverNULL(iptr);

    /* 在';'后的内容都属于注释，需要过滤 */
    char* csym = strchr(iptr -> linebuffer, ';');
    if(csym == NULL) return;

    /* 计算从';'开始后的注释字符数量并覆盖为0 */
    int cover_len = (int)((iptr -> linebuffer + iptr -> index) - csym);
    memset(csym, 0, cover_len);

    /* 刷新缓冲区长度 */
    iptr -> index = strlen(iptr -> linebuffer);
}

/* 获取文本段中，去除前后空白字符后的子字符串的首尾地址 */
static void getaddr_nospace(char* str_begin, char* str_end, char** r_begin, char** r_end)
{
    int begin = 0;
    int end = (int)(str_end - str_begin);

    /* 超出边界时视为无有效内容 */
    if(begin > end)
        goto no_content;

    /* 从前向后计算第一个非空白字符的位置 */
    while(begin < end)
    {
        if(str_begin[begin] > 32)
            break;
        begin++;
    }

    /* 从后向前计算第一个非空白字符的位置 */
    while(end > begin)
    {
        if(str_begin[end] > 32)
            break;
        end--;
    }

    /* 如果所有字符都是空白字符，则将r_begin和r_end置NULL */
    if(str_begin[begin] <= 32)
        goto no_content;

    *r_begin = str_begin + begin;
    *r_end = str_begin + end;
    return;

no_content:
    *r_begin = NULL;
    *r_end = NULL;
    return;
}

/* ! 此函数会将文本末尾的换行符消除 */
static void filter_space(struct ini* iptr)
{
    NeverNULL(iptr);
    int index = 0;
    char* begin = NULL;
    char* end = NULL;

    getaddr_nospace(iptr -> linebuffer, iptr -> linebuffer + (iptr -> index - 1), &begin, &end);
    
    /* 如果所有字符都是空白字符，则将缓冲区清0 */
    if(begin == NULL)
    {
        memset(iptr -> linebuffer, 0, INI_LIMIT_LINE);
        iptr -> index = 0;
        return;
    }
   
    /* 将非空白字符移动到缓冲区最前方 */
    while(begin <= end)
    {
        iptr -> linebuffer[index] = *begin;
        index++;
        begin++;
    }

    /* 刷新缓冲区长度 */
    iptr -> index = index;

    /* 覆盖多余字符为0 */
    memset(iptr -> linebuffer + iptr -> index, 0, INI_LIMIT_LINE - iptr -> index);
}

static void free_pair(struct ini_pair* pairptr)
{
    NeverNULL(pairptr);
    free(pairptr -> key);
    free(pairptr -> val);
    free(pairptr);
}

static void free_sect(struct ini_sect* sectptr)
{
    NeverNULL(sectptr);

    struct ini_pair* current;
    current = sectptr -> pairs;

    while(current != NULL)
    {
        ini_pair* next = current -> nextpair;
        free_pair(current);
        current = next; 
    }

    free(sectptr -> name);
    free(sectptr);
}

static void put_pair(struct ini* iptr, struct ini_pair* pairptr)
{
    NeverNULL(iptr);
    NeverNULL(pairptr);
    NeverNULL(iptr -> sections -> prevsect);

    struct ini_sect* sect_current = iptr -> sections -> prevsect;
    if(sect_current -> pairs == NULL)
    {
        pairptr -> prevpair = pairptr;
        pairptr -> nextpair = NULL;
        sect_current -> pairs = pairptr;
        return;
    }

    struct ini_pair* pair_last = sect_current -> pairs -> prevpair;
    pair_last -> nextpair = pairptr;
    pairptr -> prevpair = pair_last;
    pairptr -> nextpair = NULL;
    sect_current -> pairs -> prevpair = pairptr;
}

static CFG_ERRNO handle_pair(struct ini* iptr)
{
    NeverNULL(iptr);

    struct ini_pair* pairptr;
    char* addr_line;
    char* addr_start;
    char* addr_end;
    char* mem_key;
    char* mem_val;
    int buf_index = 0;
    int copy_len = 0;

    /* 检查是否存在section */
    if(iptr -> sections == NULL) 
        return CFG_ERR_BADTEXT;                        //在没有解析到section的情况下出现了pair

    /* 解析key */
    buf_index = iptr -> index;
    addr_line = iptr -> linebuffer;
    char* esym = strchr(addr_line, '=');
    if(esym == addr_line) return CFG_ERR_BADTEXT;       //不存在key

    getaddr_nospace(addr_line, esym - 1, &addr_start, &addr_end);
    if(addr_start == NULL) return CFG_ERR_BADTEXT;      //key全为空白字符

    copy_len = ((int)(addr_end - addr_start)) + 1;
    mem_key = (char*) calloc(copy_len + 1, 1);
    if(mem_key == NULL) return CFG_ERR_OOM;              //内存分配失败
    memcpy(mem_key, addr_start, copy_len);

    /* 解析value */
    getaddr_nospace(esym + 1, addr_line + buf_index, &addr_start, &addr_end);
    if(addr_start == NULL)
    {
        mem_val = (char*) calloc(1, 1);                     //仅分配一个'\0'空间
        if(mem_val == NULL) goto err_oom_val;            //内存分配失败
    }
    else
    {
        copy_len = ((int)(addr_end - addr_start)) + 1;
        mem_val = (char*) calloc(copy_len + 1, 1);       //多余的1是为'\0'分配
        if(mem_val == NULL) goto err_oom_val;            //内存分配失败
        memcpy(mem_val, addr_start, copy_len);
    }

    /* 保存pair */
    pairptr = (struct ini_pair*) calloc(1, sizeof(struct ini_pair));
    if(pairptr == NULL) goto err_oom_pair;              //内存分配失败
    pairptr -> key = mem_key;
    pairptr -> val = mem_val;
    put_pair(iptr, pairptr);
    return CFG_ERR_NONE;

err_oom_val:
    free(mem_key);
    return CFG_ERR_OOM;

err_oom_pair:
    free(mem_key);
    free(mem_val);
    return CFG_ERR_OOM;
}

static void calc_loca(const char* name,  unsigned int mapsize, int* r_byteloc, int* r_bitloc)
{
    NeverNULL(name);
    NeverNULL(r_byteloc);
    NeverNULL(r_bitloc);
    if(mapsize == 0) return;

    /* 计算字符数值总和 */
    int index = 0;
    unsigned int csum = 0;
    for(; name[index] != '\0'; index++)
        csum += name[index];
    
    /* 根据总和计算bit位置 */
    int bitloc = csum % (mapsize * BITWISE);
    int byteloc = bitloc / 8;
    bitloc = bitloc % BITWISE;

    *r_byteloc = byteloc;
    *r_bitloc = bitloc;
}

static void set_bit(char* bitmap, unsigned int mapsize, const char* name)
{
    NeverNULL(bitmap);
    NeverNULL(name);
    if(mapsize == 0) return;

    char* bit;
    int bitloc;
    int byteloc;

    /* 计算byte和bit位置 */
    calc_loca(name, mapsize, &byteloc, &bitloc);

    /* 设置bit */
    bit = &(bitmap[byteloc]);
    *bit = (*bit) | (1 << bitloc);
}

static void put_sect(struct ini* iptr, struct ini_sect* sectptr)
{
    NeverNULL(iptr);
    NeverNULL(sectptr);

    if(iptr -> sections == NULL)
    {
        sectptr -> prevsect = sectptr;
        sectptr -> nextsect = NULL;
        iptr -> sections = sectptr;
        return;
    }

    struct ini_sect* sect_last = iptr -> sections -> prevsect;
    sect_last -> nextsect = sectptr;
    sectptr -> prevsect = sect_last;
    sectptr -> nextsect = NULL;
    iptr -> sections -> prevsect = sectptr;
}

static CFG_ERRNO handle_sect(struct ini* iptr)
{
    NeverNULL(iptr);

    struct ini_sect* sectptr;
    char* addr_start;
    char* addr_end;
    char* addr_left;
    char* addr_right;
    char* addr_line = iptr -> linebuffer;
    char* mem_name;
    int line_len = iptr -> index;
    int copy_len = 0;

    addr_left = strchr(addr_line, '[');
    addr_right = strchr(addr_line, ']');
    if(addr_right == NULL) return CFG_ERR_BADTEXT;                 //key格式错误

    getaddr_nospace(addr_left + 1, addr_right - 1, &addr_start, &addr_end);
    if(addr_start == NULL) return CFG_ERR_BADTEXT;                 //key格式错误

    /* 计算name长度，复制name */
    copy_len = ((int)(addr_end - addr_start)) + 1;
    mem_name = (char*) calloc(copy_len + 1, 1);
    if(mem_name == NULL) return CFG_ERR_OOM;                       //内存分配失败
    memcpy(mem_name, addr_start, copy_len);

    /* 创建section */
    sectptr = (struct ini_sect*) calloc(1, sizeof(struct ini_sect));
    if(sectptr == NULL) goto err_oom_sect;
    sectptr -> name = mem_name;
    
    /* 添加section，同时设置bit */
    put_sect(iptr, sectptr);
    set_bit(iptr -> bitmap, iptr -> mapsize, sectptr -> name);
    return CFG_ERR_NONE;

err_oom_sect:
    free(mem_name);
    return CFG_ERR_OOM;    
}

static CFG_ERRNO handle_line(struct ini* iptr)
{
    NeverNULL(iptr);
    CFG_ERRNO err;

    if(iptr -> index == 0) return CFG_ERR_NONE;
    else if(strchr(iptr -> linebuffer, '[') != NULL)
        err = handle_sect(iptr);
    else if(strchr(iptr -> linebuffer, '=') != NULL)
        err = handle_pair(iptr);
    else
        err = CFG_ERR_BADTEXT;

    /* 处理完一行后，重置缓冲区 */
    memset(iptr -> linebuffer, 0, INI_LIMIT_LINE);
    iptr -> index = 0;

    return err;
}

static CFG_BOOL filter_bitmap(char* bitmap, unsigned int mapsize, const char* name)
{
    NeverNULL(bitmap);
    NeverNULL(name);
    if(mapsize == 0) return CFG_BOOL_TRUE;

    char* bit;
    int bitloc;
    int byteloc;

    calc_loca(name, mapsize, &byteloc, &bitloc);
    bit = &(bitmap[byteloc]);

    if(*bit & (1 << bitloc))
        return CFG_BOOL_TRUE;        //返回true时，section可能不存在
    return CFG_BOOL_FLASE;           //返回false时，section一定不存在
}

static CFG_BOOL find_pair(const char* key, struct ini_sect* head, struct ini_pair** r_pair)
{
    NeverNULL(key);
    NeverNULL(head);
    NeverNULL(r_pair);

    struct ini_pair* current;
    current = head -> pairs;

    while (current != NULL)
    {
        if(strcmp(current -> key, key) == 0)
        {
            *r_pair = current;
            return CFG_BOOL_TRUE;
        }

        current = current -> nextpair;
    }

    return CFG_BOOL_FLASE;
}

//--------------------interface--------------------
CFG_ERRNO ini_start(struct ini* iptr, unsigned int mapsize)
{
    if(iptr == NULL) return CFG_ERR_NULLPTR;
    memset(iptr, 0, sizeof(struct ini));

    CFG_ERRNO err;
    if((err = init_bitmap(iptr, mapsize))) return err;

    return CFG_ERR_NONE;
}

CFG_ERRNO ini_parse(struct ini* iptr, const char* str)
{
    if(iptr == NULL || str == NULL) return CFG_ERR_NULLPTR;
    
    int index = 0;
    CFG_ERRNO err;

    while(str[index] != '\0')
    {
        if((err = copy_line(iptr, str, &index))) return err;
        if(!has_line(iptr -> linebuffer)) break;
        filter_comments(iptr);
        filter_space(iptr);
        if((err = handle_line(iptr))) return err;
    }

    return CFG_ERR_NONE;
}

CFG_ERRNO ini_flush(struct ini* iptr)
{
    if(iptr == NULL) return CFG_ERR_NULLPTR;
    return ini_parse(iptr, "\n");
}

CFG_BOOL ini_getValue(struct ini* iptr, const char* name, const char* key, char** r_value)
{
    if(iptr == NULL || name == NULL || key == NULL) return CFG_BOOL_FLASE;

    struct ini_pair* target = NULL;
    struct ini_sect* head = iptr -> sections;
    
    if(!filter_bitmap(iptr -> bitmap, iptr -> mapsize, name))
        goto no_value;

    while(head != NULL)
    {
        if(strcmp(head -> name, name) == 0)
        {
            if(find_pair(key, head, &target))
            {
                if(r_value != NULL)
                    *r_value = target -> val;
                return CFG_BOOL_TRUE;
            }
            goto no_value;
        }

        head = head -> nextsect;
    }

no_value:
    *r_value = NULL;
    return CFG_BOOL_FLASE;
}

CFG_ERRNO ini_end(struct ini* iptr)
{
    if(iptr == NULL) return CFG_ERR_NULLPTR;

    struct ini_sect* current;
    current = iptr -> sections;

    while(current != NULL)
    {
        struct ini_sect* next = current -> nextsect;
        free_sect(current);
        current = next;
    }

    free(iptr -> bitmap); 
    memset(iptr, 0, sizeof(struct ini));
    return CFG_ERR_NONE;
}

CFG_ERRNO ini_beginSects(struct ini* iptr)
{
    if(iptr == NULL) return CFG_ERR_NULLPTR;
    iptr -> cursor = iptr -> sections;
    return CFG_ERR_NONE;
}

CFG_BOOL ini_nextSect(struct ini* iptr, struct ini_sect** r_sect, char** r_name)
{
    if(iptr == NULL) return CFG_BOOL_FLASE;
    if(iptr -> cursor == NULL) return CFG_BOOL_FLASE;

    /* 获取下一个section及名称 */
    if(r_sect != NULL)
        *r_sect = iptr -> cursor;
    if(r_name != NULL)
        *r_name = iptr -> cursor -> name;

    /* 更新下一个section*/
    iptr -> cursor = iptr -> cursor -> nextsect;
    
    return CFG_BOOL_TRUE;
}

CFG_ERRNO ini_beginPairs(struct ini_sect* sect)
{
    if(sect == NULL) return CFG_ERR_NULLPTR;
    sect -> cursor = sect -> pairs;
    return CFG_ERR_NONE;
}

CFG_BOOL ini_nextPair(struct ini_sect* sect, struct ini_kv* kv)
{
    if(sect == NULL) return CFG_BOOL_FLASE;
    if(sect -> cursor == NULL) return CFG_BOOL_FLASE;

    /* 获取下一个pair的key和value */
    if(kv != NULL)
    {
        kv -> key = sect -> cursor -> key;
        kv -> val = sect -> cursor -> val;
    }

    /* 更新下一个pair */
    sect -> cursor = sect -> cursor -> nextpair;
    
    return CFG_BOOL_TRUE;
}

CFG_ERRNO ini_getProperty(struct ini* iptr, ini_propname name, void* mem)
{
    if(iptr == NULL || mem == NULL) return CFG_ERR_NULLPTR;

    switch(name)
    {
        case INI_PROP_LINECNT:
            NOP;
            int* linecnt = (int*) mem;
            *linecnt = iptr -> prop.linecnt;
            return CFG_ERR_NONE;
        default:
            return CFG_ERR_BADOPT;
    }
}