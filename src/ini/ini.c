#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include "cfg/cfg.h"
#include "cfg/ini.h"

#define NeverNULL(ptr) assert(ptr != NULL)

//--------------------private--------------------


//--------------------interface--------------------
CFG_ERRNO ini_start(struct ini* ini, unsigned int mapsize)
{
    if(ini == NULL) return CFG_ERR_NULLPTR;
    memset(ini, 0, sizeof(struct ini));
    
    char* mem = (char*) calloc(INI_LIMIT_LINE);
    if(mem == NULL) return CFG_ERR_OOM;
    ini -> bitmap = mem;

    CFG_ERRNO err;
    if((err = init_bitmap(ini))) goto err_bitmap;
    
    return CFG_ERR_NONE;

err_bitmap:
    free(ini -> linebuffer);
    ini -> linebuffer = NULL;
    return err;
}

CFG_ERRNO ini_parse(struct ini* ini, const char* str)
{
    if(ini == NULL || str == NULL) return CFG_ERR_NULLPTR;
    
    int index = 0;
    char* buffer = ini -> linebuffer;
    index = copy_line(buffer, str, index);
    while(has_line(ini))
    {
        if(filter_comments(buffer)) continue;
        handle_line(ini);                                     //core!!!
        copy_line(buffer, str, index);
    }

    return CFG_ERR_NONE;
}

CFG_ERRNO ini_final(struct ini* ini)
{
    if(ini == NULL) return CFG_ERR_NULLPTR;
    ini -> linebuffer[ini -> index] = '\n';
    return ini_parse(ini, "");
}

CFG_BOOL ini_getValue(struct ini* ini, const char* name, const char* key, const char** value)
{
    if(ini == NULL || name == NULL || key == NULL) return CFG_BOOL_FLASE;

    ini_pair* target = NULL;
    ini_sect* head = ini -> sections;
    
    while(head != NULL)
    {
        if(find_pair(key, head, &target))
        {
            value = target -> val;
            return CFG_BOOL_TRUE;
        }

        head = head -> nextsect;
    }
}

CFG_ERRNO ini_free(struct ini* ini)
{
    if(ini == NULL) return CFG_ERR_NULLPTR;
    free(ini -> bitmap);    
    free(ini -> linebuffer);
    free_sect(ini -> sections);
    memset(ini, 0, sizeof(struct ini));
    return CFG_ERR_NONE;
}

CFG_ERRNO ini_beginSects(struct ini* ini);

CFG_BOOL ini_nextSect(struct ini* ini, struct ini_sect** sect);

CFG_ERRNO ini_beginPairs(struct ini_sect* sect);

CFG_BOOL ini_nextPair(struct ini_sect* sect, struct ini_kv* kv);
