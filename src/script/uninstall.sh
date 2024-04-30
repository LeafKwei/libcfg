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

function rmrf()
{
    ret_rmrf=no
    target=$1
    echo -n "do you want to remove anything in '${target}'(yes/no)?: "
    read usrin
    if [ "$usrin" == "yes" ]; then
        rm -rf ${target}
        ret_rmrf=yes
    fi
}

#-----------main------------
if [ -z "${_PATH_USRLIB}" ]; then
    echo "[ERROR] no such directroy, please check env 'PATH_LIBCFG'"
    exit
fi

rmrf ${_PATH_USRLIB}
if [ "$ret_rmrf" == no ]; then
    exit
fi

rm -f ${_PATH_LD}/${_CONFNAME}
ldconfig