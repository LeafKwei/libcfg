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
        {
		perror("failed to open test.ini"); 
		exit(1);
	}

	char* mem = (char*) malloc(65535);
	if(mem == NULL)
        {
		perror("failed to allocate memery"); 
		exit(1);
	}

	while(!feof(stream))
	{
    		memset(mem, 0, 65535);
    		fread(mem, 65534, 1, stream);
    		if((err = ini_parse(&iobj, mem)))
        		fprintf(stderr, "failed to parse test.ini: %s, line: %d\n",
                        	cfg_strerror(err),
                        	ini_getProperty(&iobj, INI_PROP_LINECNT, &linecnt));
	}

	ini_flush(&iobj);

	/* readme */
	printf("---------------readme---------------\n");
	char* value;
	if(ini_getValue(&iobj, "USER", "userName", &value))
		printf("userName = %s\n", value);		

	/* getValueFrom */
	printf("---------------getValueFrom---------------\n");
	struct ini_sect* section;
	if(ini_getSection(&iobj, "USER", &section))
	{
		if(ini_getValueFrom(section, "userLevel", &value))
			printf("userLevel = %s\n", value);
	}

	if((err = ini_end(&iobj)))
        	fprintf(stderr, "failed to end ini: %s, line: %d\n",
                        cfg_strerror(err),
                        ini_getProperty(&iobj, INI_PROP_LINECNT, &linecnt));
	free(mem);
	fclose(stream);
}
