_CC=gcc
_INCL=-I ../../include
_LINK=
_FLAG=-g
_PIC=-fPIC

# complie as static
static: opt.c
	$(_CC) -c opt.c $(_INCL) $(_LINK) $(_FLAG)

#complie as share
dynamic: opt.c
	$(_CC) -c opt.c $(_INCL) $(_LINK) $(_FLAG) $(_PIC)

clean: 
	rm -f *.o