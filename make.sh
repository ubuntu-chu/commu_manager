#!/bin/sh

CROSS_COMPILE_NAME=arm-none-linux-gnueabi-
#CROSS_COMPILE_NAME= 

INSTALL_PATH_LIB_NAME=/home/barnard/work/commu_manager/manager/lib
INSTALL_PATH_BIN_NAME=/home/barnard/work/commu_manager/manager/bin

SRC_DIR="tinyxml parse utils reactor/muduo/base reactor/muduo/net zygote commout"
CUR_PWD=`pwd`

for dir in ${SRC_DIR}
do
	echo ""
	echo "cd ${dir}/ to make"
	echo ""

	cd ${dir}
	if [ ${dir} = "zygote" -o ${dir} = "commout" ]; then
		INSTALL_PATH_NAME=${INSTALL_PATH_BIN_NAME}
	else
		INSTALL_PATH_NAME=${INSTALL_PATH_LIB_NAME}
	fi
	make CROSS_COMPILE=${CROSS_COMPILE_NAME}
	if [ 0 -ne $? ]; then
		cd ${CUR_PWD}
		exit 1
	fi
	make install INSTALL_PATH=${INSTALL_PATH_NAME}
	if [ 0 -ne $? ]; then
		cd ${CUR_PWD}
		exit 2
	fi
	cd ${CUR_PWD}
done

