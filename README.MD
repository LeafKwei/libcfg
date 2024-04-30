# libcfg文档

## PART0---libcfg

**2024/04/30(1.0.0)**: libcfg是一个以提供对各类配置文件的解析功能为目的的C函数库，目前暂时仅支持对*.ini文件进行解析。



## PART1---cfg/ini

### 简介

cfg/ini模块提供了对*.ini文件的解析功能，该模块以行为单位将ASCII编码的文本解析为ini配置项。虽然cfg/ini模块以行为单位对文本进行解析，但允许输入的文本是碎片化的，这意味着每次输入的文本可以是一行文本的一部分，cfg/ini会将输入的文本保存到内部的缓冲区中，并自动检查缓冲区中的文本是否是完整的一行(通过检查换行符实现)，在文本满足一行后，对文本进行解析。

由于使用了缓冲区，cfg/ini不支持解析单行超过1023个字符(ASCII编码，包括注释和换行符)的文本，这个限制可以通过修改源码中的include/ini.h::INI_LIMIT_LINE来扩展。

### 示例

以下方的test.ini文件为例，我们将以此文件为例介绍cfg/ini功能。在其中，`[USER]`及其后的内容被称为**section**，`USER`被称为section的**name**，`userName = admin`被称为**pair**，`userName`被称为pair的**key**，而`admin`被称为pair的**value**。

```
[USER]
userName = admin  ;this is comment
userLevel = 0

[Properties]
time = 100
maxCount = 8
```

**struct ini**是cfg/ini的核心，在解析一个ini配置文件前，我们需要创建一个该结构体:

```c
struct ini iobj;
```

创建后，我们需要使用ini_start函数对其进行初始化，该函数还需提供一个**unsigned int**参数bitsize，用于初始化**struct ini**内部的位图，位图可以在后续获取ini配置项时，尽早识别到那些不存在的section。

```c
int linecnt;
CFG_ERRNO err;

if((err = ini_start(&iobj, 16))) 
	fprintf(stderr, "failed to init iobj: %s, line: %d\n", 
		cfg_strerror(err), 
		ini_getProperty(&iobj, INI_PROP_LINECNT, &linecnt));
```

bitsize以bit为单位，每8个占用一个char的大小。可以根据test.ini中section的数量来决定bitsize的大小。ini_start函数初始化完成后会返回一个CFG_ERRNO类型的结果，正常情况下该值为0(对应宏CFG_ERR_NONE)，其他情况下是大于0的错误码，可以使用cfg_strerror获取错误码对应的字面含义(错误码可见include/cfg.h::cfg_errno)。ini_getProperty函数可获取struct ini内部的一些属性，可见include/ini.h::ini_propname。

在初始化完成后，就可以使用ini_parse函数对test.ini文件进行解析，由于该函数只接受char数组作为输入，因此需要开发者实现文件读取功能，然后将文件数据作为输入:

```c
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
```

由于cfg/ini是通过识别换行符`\n`来判断文本是否是完整的一行，而文件中的最后一行文本末尾可能不存在换行符，因此需要使用ini_flush函数来显式的追加换行符，该函数在当前版本等同于调用`ini_parse(&iobj, "\n")`。

在成功完成解析后，使用ini_getValue函数来获取配置值:

```
char* value;
if((err = ini_getValue(&iobj, "USER", "userName", &value)))
	fprintf(stderr, "failed to get value: %s, line: %d\n", 
			cfg_strerror(err), 
			ini_getProperty(&iobj, INI_PROP_LINECNT, &linecnt));
printf("userNmae = %s\n", value);
```

最后，使用ini_end函数释放struct ini中的资源即可:

```
if((err = ini_end(&iobj)))
	fprintf(stderr, "failed to end ini: %s, line: %d\n", 
			cfg_strerror(err), 
			ini_getProperty(&iobj, INI_PROP_LINECNT, &linecnt));

free(mem);
fclose(stream);
```

考虑到某些情况下需要使用到遍历功能，cfg/ini也提供了对*.ini文件遍历的接口，可以参考src/sample/ini/iterate.c，在src/sample/ini中也还有一些其他情况下的测试代码。





