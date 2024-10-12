_CC=gcc
_INCL=-I ../../include
_LINK=
_FLAG=-g
_PIC=-fPIC

# complie as static
static: ini.c
	$(_CC) -c ini.c $(_INCL) $(_LINK) $(_FLAG)

#complie as share
dynamic: ini.c
	$(_CC) -c ini.c $(_INCL) $(_LINK) $(_FLAG) $(_PIC)

clean: 
	rm -f *.o

