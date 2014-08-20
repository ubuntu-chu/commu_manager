#!/bin/sh

#指定交叉编译器   若其值为空 则代表编译成PC版本
#否则为嵌入式版本
CROSS_COMPILE_NAME=arm-none-linux-gnueabi-
#CROSS_COMPILE_NAME= 

#应用程序、库安装到根文件系统中的位置
INSTALL_DST_PATH=/home/barnard/work/board_9G25/rootfs_rfid/root/project/
#应用程序、库安装位置
INSTALL_PATH=/home/barnard/work/commu_manager/manager
#库安装位置
INSTALL_PATH_LIB_NAME=${INSTALL_PATH}/lib
#应用程序安装位置
INSTALL_PATH_BIN_NAME=${INSTALL_PATH}/bin

#源码路径 程序会依次cd 源码目录下 执行make后 再返回到源码顶层目录
SRC_DIR="tinyxml parse utils reactor/muduo/base reactor/muduo/net zygote commout"
CUR_PWD=`pwd`

#循环执行命令
for dir in ${SRC_DIR}
do
	echo ""
	echo "cd ${dir}/ to make"
	echo ""

	cd ${dir}
	#zygote commout目录为可执行文件源码
	if [ ${dir} = "zygote" -o ${dir} = "commout" ]; then
		INSTALL_PATH_NAME=${INSTALL_PATH_BIN_NAME}
	#其他目录下 为库文件源码
	else
		INSTALL_PATH_NAME=${INSTALL_PATH_LIB_NAME}
	fi
	#执行make 命令   向其传入CROSS_COMPILE参数
	make CROSS_COMPILE=${CROSS_COMPILE_NAME}
	#make 命令成功执行后  会返回0   此处判断make命令执行是否成功
	if [ 0 -ne $? ]; then
		#命令执行不成功， 则返回到源码顶层目录
		cd ${CUR_PWD}
		exit 1
	fi
	make install INSTALL_PATH=${INSTALL_PATH_NAME}
	if [ 0 -ne $? ]; then
		cd ${CUR_PWD}
		exit 2
	fi
	#返回到源码顶层目录
	cd ${CUR_PWD}
done

#将可执行文件、库文件 拷贝到根文件系统的相应目录下
echo "barnard" | sudo -S cp -r ${INSTALL_PATH}/* ${INSTALL_DST_PATH}



