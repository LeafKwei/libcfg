#!/bin/bash

_DW=$1
_VERSION=1.0.0
_PATH_ROOT=$2
_PATH_USRLIB=${PATH_LIBCFG}
_PATH_LD=/etc/ld.so.conf.d
_REALNAME=libcfg.so.${_VERSION}
_SONAME=libcfg.so.1
_LINKNAME=libcfg.so
_CONFNAME=libcfg.so.conf
_OBJS='cfg.o ini.o'
ret_rmrf=no

function mv_any()
{
	cd ${_PATH_ROOT}/src
	mv ./cfg/*.o ${_PATH_ROOT}/output
	mv ./ini/*.o ${_PATH_ROOT}/output
	cd ${_PATH_ROOT}/output
}

#-----------main----------
if [ $_DW = "static" ]; then
	mv_any
	ar crv libcfg.a $_OBJS
	strip libcfg.a --strip-unneeded
elif [ $_DW = "dynamic" ]; then
	mv_any
	gcc -o $_REALNAME $_OBJS -fPIC -shared -Wl,-soname,$_SONAME
fi

rm -f ${_PATH_ROOT}/output/*.o
