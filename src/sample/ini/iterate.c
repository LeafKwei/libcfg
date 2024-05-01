#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg/ini.h"

int main(void)
{
	struct ini iobj;

	int linecnt;
	CFG_ERRNO err;

	if((err = ini_start(&iobj, 16)))
        	fprintf(stderr, "failed to init iobj: %s, line: %d\n",
                	cfg_strerror(err),
 	        	ini_getProperty(&iobj, INI_PROP_LINECNT, &linecnt));

	FILE* stream;
	stream = fopen("test.ini", "r+");
	if(stream == NULL)
        	{perror("failed to open test.ini"); exit(1);}

	char* mem = (char*) malloc(65535);
	if(mem == NULL)
        	{perror("failed to allocate memery"); exit(1);}

	while(!feof(stream))
	{
    		memset(mem, 0, 65535);
    		fread(mem, 65535, 1, stream);
    		if((err = ini_parse(&iobj, mem)))
        		fprintf(stderr, "failed to parse test.ini: %s, line: %d\n",
                        	cfg_strerror(err),
                        	ini_getProperty(&iobj, INI_PROP_LINECNT, &linecnt));
	}

	ini_flush(&iobj);

	
	/* iterate */
	printf("---------------iterate---------------\n");
	char* name;
	struct ini_sect* section;
	struct ini_kv kv;
	
	if((err = ini_beginSects(&iobj)))
		fprintf(stderr, "failed to begin sections: %s\n", 
			cfg_strerror(err));
	while(ini_nextSect(&iobj, &section, &name))
	{
		printf("[%s]\n", name);
		if((err = ini_beginPairs(section)))
			fprintf(stderr, "failed to begin pairs: %s\n", 
				cfg_strerror(err));
		while(ini_nextPair(section, &kv))
		{
			printf("%s = %s\n", kv.key, kv.val);
		}
		
		printf("\n");
	}

	/* iterate2 */
	printf("---------------iterate2---------------\n");
	if(ini_getSection(&iobj, "USER", &section))
    {
        if((err = ini_beginPairs(section)))
            fprintf(stderr, "failed to begin pairs: %s, line: %d\n",
                cfg_strerror(err),
                ini_getProperty(&iobj, INI_PROP_LINECNT, &linecnt));

        printf("[USER]\n");
        while(ini_nextPair(section, &kv))
        {
            printf("%s = %s\n", kv.key, kv.val);
        }
    }
	
	if((err = ini_end(&iobj)))
        	fprintf(stderr, "failed to end ini: %s, line: %d\n",
                        cfg_strerror(err),
                        ini_getProperty(&iobj, INI_PROP_LINECNT, &linecnt));
	free(mem);
	fclose(stream);
}
