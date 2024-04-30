BINS=iterate readme

all: $(BINS)

iterate: iterate.c	
	gcc -o iterate iterate.c -I $(PATH_LIBCFG)/include -L $(PATH_LIBCFG) -lcfg 
readme: readme.c
	gcc -o readme readme.c -I $(PATH_LIBCFG)/include -L $(PATH_LIBCFG) -lcfg 
clean:
	rm -f $(BINS)  
