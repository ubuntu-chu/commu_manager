#############################################################
# Generic Makefile for C/C++ Program
#
# Description:
# ------------
#
# Make Target:
# ------------
# The Makefile provides the following targets to make:
#   $ make           compile and link
#   $ make NODEP=yes compile and link without generating dependencies
#   $ make objs      compile only (no linking)
#   $ make tags      create tags for Emacs editor
#   $ make ctags     create ctags for VI editor
#   $ make clean     clean objects and the executable file
#   $ make distclean clean objects, the executable and dependencies
#   $ make help      get the usage of the makefile
#
#===========================================================================

## Customizable Section: adapt those variables to suit your program.
##==========================================================================

# BUILD_DEBUG can be set to YES to include debugging info, or NO otherwise
BUILD_DEBUG          := YES
#应用程序、静态库、动态库名字
PROGRAM_NAME	= commout  
#cross compile tools defined 
#CROSS_COMPILE ?= arm-none-linux-gnueabi-
CROSS_COMPILE ?=

#BUILD_TYPE 编译目标类型：执行文件  静态库  动态库
BUILD_TYPE		= TYPE_EXEC
#BUILD_TYPE		= TYPE_STATIC
#BUILD_TYPE 	= TYPE_DYNAMIC

#global directory defined
TOPDIR       	= $(shell pwd)
#SRCDIR	 		= $(TOPDIR)/src
SRCDIR	 		= ./
#所引用的库目录设置 以空格分隔
LIBDIR      	= ../manager/libs
#所引用库设置
LDFLAGS    		= -lreactor_net -lreactor_base -lpthread -lrt

#包含头文件路径设置
INCLUDEDIR   	= ../reactor ../boost ../
#make install INSTALL_PATH   
#INSTALL_PATH变量可由make传递进来的参数改写
#安装路径设置
INSTALL_PATH 	= ./install/
# *.o *.d 文件存放位置
OBJDIR    		= ./objs/
#应用程序目录设置
PROGRAM_DIR 	= bin/  

#动态库版本号定义
SO_MAJOR_VER    =0
SO_MINOR_VER    =0
SO_BUILD_VER    =1

#c flags
CFLAGS    		= 

#c++ flags
CXXFLAGS  		= 

#编译C／C++程序时，共用编译标志
#DEP_OPT = $(shell if `$(CC) --version | grep "gcc" >/dev/null`; then \
#		  				echo "-MM -MP"; else echo "-M"; fi )
DEP_OPT 		= -MD
DEBUG_CPPFLAGS   := -Wall -Wno-format -g -DBULID_DEBUG -O0
RELEASE_CPPFLAGS := -Wall -Wno-unknown-pragmas -Wno-format -O3
ifeq (YES, ${BUILD_DEBUG})
	CPPFLAGS     := ${DEBUG_CPPFLAGS}
else
	CPPFLAGS     := ${RELEASE_CPPFLAGS}
endif

#编译参数设置
CPPFLAGS  		+= $(DEP_OPT) 
ifeq ($(BUILD_TYPE), TYPE_DYNAMIC)
	CPPFLAGS  	+= -fpic
endif
INCLUDE__DIR   	= $(strip $(INCLUDEDIR))
ifneq ($(INCLUDE__DIR),)
CPPFLAGS		+= $(foreach dir,$(INCLUDE__DIR),-I$(dir))
endif

#静态库设置
ARFLAGS 		= rc

#链接库设置
XLDFLAGS   		= -Xlinker "-(" $(LDFLAGS) -Xlinker "-)"
LIB__DIR   		= $(strip $(LIBDIR))
ifneq ($(LIB__DIR),)
LDLIBS		 	+= $(foreach dir,$(LIB__DIR),-L$(dir))
endif

PROGRAM__DIR   	= $(strip $(PROGRAM_DIR))
ifeq ($(BUILD_TYPE), TYPE_DYNAMIC)
	PROGRAMDIR   	= $(PROGRAM__DIR)/dynamic
	PROGRAM      	= $(strip lib$(strip $(PROGRAM_NAME)).so)
endif
ifeq ($(BUILD_TYPE), TYPE_EXEC)
	PROGRAMDIR   	= $(PROGRAM__DIR)/exec
	PROGRAM      	= $(strip $(PROGRAM_NAME))
endif
ifeq ($(BUILD_TYPE), TYPE_STATIC)
	PROGRAMDIR   	= $(PROGRAM__DIR)/static
	PROGRAM      	= $(strip lib$(strip $(PROGRAM_NAME)).a)
endif
PROGRAM_PATH 	= $(strip $(PROGRAMDIR)/$(PROGRAM))

# 判断相应目录是否存在 若不存在则建立目录
$(shell if ! [ -d $(OBJDIR) ] ; then \
		mkdir -p $(OBJDIR) ; fi)
$(shell if ! [ -d $(PROGRAMDIR) ] ; then \
		mkdir -p $(PROGRAMDIR) ; fi)
$(shell if ! [ -d $(SRCDIR) ] ; then \
		mkdir -p $(SRCDIR) ; fi)

## Implicit Section: change the following only when necessary.
##==========================================================================

# The source file types (headers excluded).
# .c indicates C source files, and others C++ ones.
SRCEXTS = .c .C .cc .cpp .CPP .c++ .cxx .cp

# The header file types.
HDREXTS = .h .H .hh .hpp .HPP .h++ .hxx .hp

AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
CPP     = $(CROSS_COMPILE)g++
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
RANLIB 	= $(CROSS_COMPILE)ranlib

#local host tools defined
CP		:= cp
RM		:= rm -f
MKDIR	:= mkdir
SED		:= sed
FIND	:= find
MKDIR	:= mkdir
XARGS	:= xargs

#tags define
ETAGS = etags
ETAGSFLAGS =

CTAGS = ctags
CTAGSFLAGS =


## Stable Section: usually no need to be changed. But you can add more.
##==========================================================================
SHELL    = /bin/sh
EMPTY    =
##定义SPACE变量的值为一个空格
SPACE    = $(EMPTY) $(EMPTY)
ifeq ($(PROGRAM),)
##CURDIR应该是SHELL中的一个标准变量 里面存放着当前的路径名称
##$(subst $(SPACE),_,$(CURDIR))将当前路径中的空格替换成_
##$(subst /,$(SPACE),$(subst $(SPACE),_,$(CURDIR))) 将当前路径中的/替换为空格
    CUR_PATH_NAMES = $(subst /,$(SPACE),$(subst $(SPACE),_,$(CURDIR)))
##$(words $(CUR_PATH_NAMES)) 到此步时 当前路径中的每个部分都是用空格作为分隔符
##words 命令用于计算字符串中的字符个数
##$(word $(words $(CUR_PATH_NAMES)),$(CUR_PATH_NAMES)) 用于提取字符串的字符 这个
##命令用于将字符串中的最后一个字符提取出来
    PROGRAM = $(word $(words $(CUR_PATH_NAMES)),$(CUR_PATH_NAMES))
##若变量PROGRAM为空  则赋值为一个默认值
    ifeq ($(PROGRAM),)
        PROGRAM = a.out
    endif
endif

#FILE' INFOMATION COLLECT
#找到srcdir下的所有子目录
VPATH 			= $(shell ls -AxR $(SRCDIR)|grep ":"|grep -v "\.svn"|tr -d ':')
SOURCEDIRS	= $(VPATH)
##找出所有的头文件
HEADERS = $(foreach d,$(SOURCEDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))
#search source file in the current dir
SOURCES 	= $(foreach d,$(SOURCEDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
SRCOBJS    	= $(addsuffix .o, $(basename $(SOURCES)))  
OBJS   		= $(subst $(SRCDIR),$(OBJDIR),$(SRCOBJS))
##DEPS为所有源文件对应的依赖文件 .d文件
DEPS    	= $(OBJS:.o=.d)
##获取所有的不以.c为结尾的源文件  filter-out的作用为列出所有不符合的文件
##对于C++ C的工程来讲 用于判断是否存在C++文件 这个也决定所使用的编译器 g++或gcc 
SRC_CXX = $(filter-out %.c,$(SOURCES))

.PHONY: all objs tags ctags clean distclean help show run target_type info strip install

# Delete the default suffixes
.SUFFIXES:

all: target_type

#for .h header files dependence
-include $(DEPS)

ifeq ($(BUILD_TYPE), TYPE_DYNAMIC)
target_type:$(OBJS)
#	@echo "generate real name"
	$(CC) -shared $^ $(CPPFLAGS) $(CFLAGS) $(XLDFLAGS) -Wl,-soname,$(PROGRAM).$(SO_MAJOR_VER) -o $(PROGRAMDIR)/$(PROGRAM).$(SO_MAJOR_VER).$(SO_MINOR_VER).$(SO_BUILD_VER) $(LDLIBS)
#	@echo "generate so name"
	@ldconfig -n $(PROGRAMDIR)
#	@echo "generate link name"
#	注意：command1;command2	和
#	command1
#	command2
#	是不一样的哦。
#
#	前面一种是command2的执行依赖command1的执行结果
#	后面一种情况就没有这个效果
	@cd $(PROGRAMDIR);\
	ln -sf $(PROGRAM).$(SO_MAJOR_VER) $(PROGRAM);\
	cd $(TOPDIR)
	@echo
	@echo "build_dir: [./$(PROGRAMDIR)]  build_type: [$(BUILD_TYPE)]"
	@echo "type \"make info\" to get info of the so librarys"
	@echo
endif

ifeq ($(BUILD_TYPE), TYPE_EXEC)
target_type :$(OBJS)
ifeq ($(SRC_CXX),)              # C program
	$(CC) $^ $(CPPFLAGS) $(CXXFLAGS) $(XLDFLAGS) -o $(PROGRAMDIR)/$(PROGRAM) $(LDLIBS) 
else                            # C++ program
	$(CPP) $^ $(CPPFLAGS) $(CXXFLAGS) $(XLDFLAGS) -o $(PROGRAMDIR)/$(PROGRAM) $(LDLIBS) 
endif
	@echo
	@echo "build_dir: [./$(PROGRAMDIR)]  build_type: [$(BUILD_TYPE)]"
	@echo "type \"make info\" to get info of the exec file"
	@echo "type \"make run\" to execute the program"
	@echo
endif

ifeq ($(BUILD_TYPE), TYPE_STATIC)
target_type:$(OBJS)
	$(AR) $(ARFLAGS) $(PROGRAMDIR)/$(PROGRAM) $^
	$(RANLIB) $(PROGRAMDIR)/$(PROGRAM) 
	@echo
	@echo "build_dir: [./$(PROGRAMDIR)]  build_type: [$(BUILD_TYPE)]"
	@echo "type \"make info\" to get info of the static librarys"
	@echo
endif

# Rules for generating object files (.o).
#----------------------------------------
objs:$(OBJS)

#生成相应的.o文件  若目标目录不存在 则先创建目标文件夹
$(OBJDIR)%.o: $(SRCDIR)%.c
	[ ! -d $(dir $@) ] & $(MKDIR) -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(OBJDIR)%.o: $(SRCDIR)%.C
	[ ! -d $(dir $@) ] & $(MKDIR) -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)%.o: $(SRCDIR)%.cc
	[ ! -d $(dir $@) ] & $(MKDIR) -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	[ ! -d $(dir $@) ] & $(MKDIR) -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)%.o: $(SRCDIR)%.CPP
	[ ! -d $(dir $@) ] & $(MKDIR) -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)%.o: $(SRCDIR)%.cxx
	[ ! -d $(dir $@) ] & $(MKDIR) -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

# Rules for generating the tags.
#-------------------------------------
tags: $(HEADERS) $(SOURCES)
	$(ETAGS) $(ETAGSFLAGS) $(HEADERS) $(SOURCES)

ctags: $(HEADERS) $(SOURCES)
	$(CTAGS) $(CTAGSFLAGS) $(HEADERS) $(SOURCES)

clean:
	$(RM) $(OBJS) $(PROGRAMDIR)/*

distclean: 
	$(RM) -r $(OBJDIR) 
	$(RM) -r $(PROGRAMDIR)
#	$(RM) -r $(PROGRAM__DIR)
#	$(FIND) $(OBJDIR) -name "*.o" -o -name "*.d" | $(XARGS) $(RM) -f

run:
	./$(PROGRAM_PATH)

info:
	ls -l $(PROGRAMDIR)
	@echo
ifeq ($(BUILD_TYPE), TYPE_DYNAMIC)
	file $(PROGRAMDIR)/$(PROGRAM).$(SO_MAJOR_VER).$(SO_MINOR_VER).$(SO_BUILD_VER)
else
	file $(PROGRAMDIR)/$(PROGRAM)
endif
	@echo

strip:
ifeq ($(BUILD_TYPE), TYPE_DYNAMIC)
	$(STRIP)  --strip-unneeded $(PROGRAMDIR)/$(PROGRAM).$(SO_MAJOR_VER).$(SO_MINOR_VER).$(SO_BUILD_VER)
else
	$(STRIP)  --strip-unneeded $(PROGRAMDIR)/$(PROGRAM)
endif

install:
	$(shell if ! [ -d $(INSTALL_PATH) ] ; then \
			mkdir -p $(INSTALL_PATH) ; fi)
	cp $(PROGRAMDIR)/* $(INSTALL_PATH)

# Show help.
help:
	@echo 'Generic Makefile for C/C++ Programs (gcmakefile) version 0.5'
	@echo 'Copyright (C)       chumojing@163.com>                      '
	@echo
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       (=make) compile and link.'
	@echo '  NODEP=yes make without generating dependencies.'
	@echo '  objs      compile only (no linking).'
	@echo '  tags      create tags for Emacs editor.'
	@echo '  ctags     create ctags for VI editor.'
	@echo '  clean     clean objects and the executable file.'
	@echo '  distclean clean objects, the executable and dependencies.'
	@echo '  show      show variables (for debug use only).'
	@echo '  help      print this message.'
	@echo '  run       run the program.'
	@echo '  info      get info of the program.'
	@echo '  strip     strip the program.'
	@echo

# Show variables (for debug use only.)
show:
	@echo 'BUILD_TYPE     :' $(BUILD_TYPE)
	@echo 'PROGRAM_DIR    :' $(PROGRAMDIR)
	@echo 'PROGRAM        :' $(PROGRAM)
	@echo 'PROGRAM_PATH   :' $(PROGRAM_PATH)
	@echo 'PROGRAMLIBS    :' $(PROGRAM)
	@echo 'PROGRAMSLIBS   :' $(PROGRAM)
	@echo 'SOURCEDIRS     :' $(SOURCEDIRS)
	@echo 'HEADERS        :' $(HEADERS)
	@echo 'SOURCES        :' $(SOURCES)
	@echo 'SRC_CXX        :' $(SRC_CXX)
	@echo 'SRC_OBJS       :' $(SRCOBJS)
	@echo 'OBJS           :' $(OBJS)
	@echo 'DEPS           :' $(DEPS)
	@echo 'DEP_OPT        :' $(DEP_OPT)
	@echo 'curr path names:' $(CUR_PATH_NAMES)
	@echo 'curr path      :' $(CURDIR)
	@echo 'SRC_DIR        :' $(SRCDIR)
	@echo 'OBJ_DIR        :' $(OBJDIR)

## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##
##############################################################
