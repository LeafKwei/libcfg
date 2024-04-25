_PATH_ROOT=$(shell pwd)

all:
	make static _PATH_ROOT=$(_PATH_ROOT) -C src
	make dynamic _PATH_ROOT=$(_PATH_ROOT) -C src

install:
	make install _PATH_ROOT=$(_PATH_ROOT) -C src/script

force:
	make force _PATH_ROOT=$(_PATH_ROOT) -C src/script

uninstall:
	make uninstall _PATH_ROOT=$(_PATH_ROOT) -C src/script
	
clean:
	make clean _PATH_ROOT=$(_PATH_ROOT) -C src
	rm -f $(_PATH_ROOT)/output/*