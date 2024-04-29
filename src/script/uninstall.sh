#!/bin/bash

_DW=$1
_VERSION=1.0.0
_PATH_ROOT=$2
_PATH_USRLIB=/opt/lib
_PATH_LD=/etc/ld.so.conf.d
_REALNAME=libcfg.so.${_VERSION}
_SONAME=libcfg.so.1
_LINKNAME=libcfg.so
_CONFNAME=libcfg.so.conf
_OBJS='cfg.o ini.o'

rm -rf ${_PATH_USRLIB}
rm -f ${_PATH_LD}/${_CONFNAME}
ldconfig