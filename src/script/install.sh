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

function setld()
{
    if [ ! "$1" = "force" ] && [ -e ${_PATH_LD}/${_CONFNAME} ]; then 
        echo "[ERROR] intall: there has a same file '${_CONFNAME}' in '${_PATH_LD}', please ensure that is not a system file, if not, redo by 'make force' "
        exit
    fi

    rm -f ${_PATH_LD}/${_CONFNAME}
    echo "$_PATH_USRLIB" > ${_PATH_LD}/${_CONFNAME}
    ldconfig
}

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

function initdirs()
{
    if [ -d $_PATH_USRLIB ]; then
        rmrf ${_PATH_USRLIB}/*
        return
    fi

    mkdir -p $_PATH_USRLIB
}

function movelib()
{
    cd ${_PATH_ROOT}/output
    cp libcfg.a $_PATH_USRLIB
    cp $_REALNAME $_PATH_USRLIB
    setld "$1"
    #cd $_PATH_USRLIB
    #ln -s "`pwd`"/$_SONAME $_LINKNAME
}

function moveinc()
{
    cp -r ${_PATH_ROOT}/include ${_PATH_USRLIB}
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

if [ "$1" = "force" ]; then
    initdirs
    movelib "force"
    moveinc
    exit
fi

initdirs
movelib ""
moveinc