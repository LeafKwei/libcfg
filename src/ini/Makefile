_CC=gcc
_INCL=-I ../../include
_LINK=
_FLAG=
_PIC=-fPIC

# complie as static
static: ini.c ini_plugin.c
	$(_CC) -c ini.c ini_plugin.c $(_INCL) $(_LINK) $(_FLAG)

#complie as share
dynamic: ini.c ini_plugin.c
	$(_CC) -c ini.c ini_plugin.c $(_INCL) $(_LINK) $(_FLAG) $(_PIC)

clean: 
	rm -f *.o

