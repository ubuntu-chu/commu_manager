#!/bin/sh

CROSS_COMPILE_NAME=arm-none-linux-gnueabi-
#CROSS_COMPILE_NAME= 

INSTALL_PATH_NAME=/home/barnard/work/commu_manager/manager/lib

SRC_DIR="tinyxml parse utils reactor/muduo/net reactor/muduo/net zygote commout"
CUR_PWD=`pwd`

for dir in ${SRC_DIR}
do
	echo ""
	echo "cd ${dir}/ to make"
	echo ""

	cd ${dir}
	make distclean;\
	resu=$?;
	cd ${CUR_PWD}
	if [ 0 -ne $resu ]; then
		exit 1
	fi
done

