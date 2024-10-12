_CC=gcc
_INCL=-I ../../include
_LINK=
_FLAG=-g
_PIC=-fPIC

# complie as static
static: cfg.c
	$(_CC) -c cfg.c $(_INCL) $(_LINK) $(_FLAG)

#complie as share
dynamic: cfg.c
	$(_CC) -c cfg.c $(_INCL) $(_LINK) $(_FLAG) $(_PIC)

clean: 
	rm -f *.o

