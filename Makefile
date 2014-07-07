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

PROGRAM_DIR := bin    
PROGRAM_NAME:= application  
#cross compile tools defined 
CROSS_COMPILE ?= 


#global directory defined
TOPDIR       = $(shell pwd)
SRCDIR	 	= $(TOPDIR)/src
#应用程序所引用的库目录
LIBDIR      = $(TOPDIR)/lib
INCLUDEDIR   = $(TOPDIR)/include
# *.o *.d 文件存放位置
OBJDIR    = $(TOPDIR)/objs

# The options used in linking as well as in any direct use of ld.
LDFLAGS    = 

#c flags
CFLAGS    	= -g -O0

#c++ flags
CXXFLAGS  = -g -O0 

#编译C／C++程序时，共用编译标志
#DEP_OPT = $(shell if `$(CC) --version | grep "gcc" >/dev/null`; then \
#		  				echo "-MM -MP"; else echo "-M"; fi )
DEP_OPT 	= -MD
CPPFLAGS  	:= -Wall $(DEP_OPT) $(foreach dir,$(INCLUDEDIR),-I$(dir))
#
#
ARFLAGS = rc

XLDFLAGS   = -Xlinker "-(" $(LDFLAGS) -Xlinker "-)"
LDLIBS		 += -L $(LIBDIR) 

# The executable file name.
# If not specified, current directory name or `a.out' will be used.
PROGRAMDIR   = $(strip $(PROGRAM_DIR))
PROGRAM      = $(strip $(PROGRAM_NAME))
PROGRAMLIBS  = $(strip lib$(PROGRAM).a)
PROGRAMSLIBS = $(strip lib$(PROGRAM).so)
PROGRAM_PATH = $(strip $(PROGRAMDIR)/$(PROGRAM))

# 判断相应目录是否存在 若不存在则建立目录
$(shell if ! [ -d $(OBJDIR) ] ; then \
		mkdir $(OBJDIR) ; fi)
$(shell if ! [ -d $(PROGRAMDIR) ] ; then \
		mkdir $(PROGRAMDIR) ; fi)
$(shell if ! [ -d $(LIBDIR) ] ; then \
		mkdir $(LIBDIR) ; fi)
$(shell if ! [ -d $(INCLUDEDIR) ] ; then \
		mkdir $(INCLUDEDIR) ; fi)
$(shell if ! [ -d $(SRCDIR) ] ; then \
		mkdir $(SRCDIR) ; fi)

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


.PHONY: all objs tags ctags clean distclean help show exec

# Delete the default suffixes
.SUFFIXES:

all: $(PROGRAM)

#for .h header files dependence
-include $(DEPS)

$(PROGRAM) :$(OBJS)
ifeq ($(SRC_CXX),)              # C program
	$(CC) $(subst $(SRCDIR),$(OBJDIR),$^) $(CPPFLAGS) $(CFLAGS) $(XLDFLAGS) -o $(PROGRAMDIR)/$@ $(LDLIBS) 
else                            # C++ program
	$(CPP) $(subst $(SRCDIR),$(OBJDIR),$^) $(CPPFLAGS) $(CXXFLAGS) $(XLDFLAGS) -o $(PROGRAMDIR)/$@ $(LDLIBS) 
endif
	#$(STRIP)  --strip-unneeded $(PROGRAM)

$(PROGRAMLIBS) :$(OBJS)
	$(AR) $(ARFLAGS) $(PROGRAMDIR)/$@ $(OBJS)
	$(RANLIB) $(PROGRAMDIR)/$@ $(OBJS)

$(PROGRAMSLIBS) :$(OBJS)
	$(CC) -shared $(subst $(SRCDIR),$(OBJDIR),$^) $(CPPFLAGS) $(CFLAGS) $(XLDFLAGS) -o $(PROGRAMDIR)/$@ $(LDLIBS)

# Rules for generating object files (.o).
#----------------------------------------
objs:$(OBJS)

#生成相应的.o文件  若目标目录不存在 则先创建目标文件夹
$(OBJDIR)%.o: $(SRCDIR)%.c
	[ ! -d $(dir $(subst $(SRCDIR),$(OBJDIR),$@)) ] & $(MKDIR) -p $(dir $(subst $(SRCDIR),$(OBJDIR),$@))
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $(subst $(SRCDIR),$(OBJDIR),$@) -c $<

$(OBJDIR)%.o: $(SRCDIR)%.C
	[ ! -d $(dir $(subst $(SRCDIR),$(OBJDIR),$@)) ] & $(MKDIR) -p $(dir $(subst $(SRCDIR),$(OBJDIR),$@))
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $(subst $(SRCDIR),$(OBJDIR),$@) -c $<

$(OBJDIR)%.o: $(SRCDIR)%.cc
	[ ! -d $(dir $(subst $(SRCDIR),$(OBJDIR),$@)) ] & $(MKDIR) -p $(dir $(subst $(SRCDIR),$(OBJDIR),$@))
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $(subst $(SRCDIR),$(OBJDIR),$@) -c $<

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	[ ! -d $(dir $(subst $(SRCDIR),$(OBJDIR),$@)) ] & $(MKDIR) -p $(dir $(subst $(SRCDIR),$(OBJDIR),$@))
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $(subst $(SRCDIR),$(OBJDIR),$@) -c $<

$(OBJDIR)%.o: $(SRCDIR)%.CPP
	[ ! -d $(dir $(subst $(SRCDIR),$(OBJDIR),$@)) ] & $(MKDIR) -p $(dir $(subst $(SRCDIR),$(OBJDIR),$@))
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $(subst $(SRCDIR),$(OBJDIR),$@) -c $<

$(OBJDIR)%.o: $(SRCDIR)%.cxx
	[ ! -d $(dir $(subst $(SRCDIR),$(OBJDIR),$@)) ] & $(MKDIR) -p $(dir $(subst $(SRCDIR),$(OBJDIR),$@))
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) -o $(subst $(SRCDIR),$(OBJDIR),$@) -c $<

# Rules for generating the tags.
#-------------------------------------
tags: $(HEADERS) $(SOURCES)
	$(ETAGS) $(ETAGSFLAGS) $(HEADERS) $(SOURCES)

ctags: $(HEADERS) $(SOURCES)
	$(CTAGS) $(CTAGSFLAGS) $(HEADERS) $(SOURCES)

clean:
	$(RM) $(OBJS) $(PROGRAM) $(PROGRAMLIBS) $(PROGRAMSLIBS) 

distclean: clean
	$(RM) -r $(OBJDIR) 
#	$(FIND) $(OBJDIR) -name "*.o" -o -name "*.d" | $(XARGS) $(RM) -f

exec:
	./$(PROGRAM_PATH)

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
	@echo '  exec      exec the program
	@echo

# Show variables (for debug use only.)
show:
	@echo 'PROGRAM_DIR    :' $(PROGRAM_DIR)
	@echo 'PROGRAM        :' $(PROGRAM)
	@echo 'PROGRAM_PATH   :' $(PROGRAM_PATH)
	@echo 'PROGRAMLIBS    :' $(PROGRAMLIBS)
	@echo 'PROGRAMSLIBS   :' $(PROGRAMSLIBS)
	@echo 'SOURCEDIRS     :' $(SOURCEDIRS)
	@echo 'HEADERS        :' $(HEADERS)
	@echo 'SOURCES        :' $(SOURCES)
	@echo 'SRC_CXX        :' $(SRC_CXX)
	@echo 'OBJS           :' $(OBJS)
	@echo 'DEPS           :' $(DEPS)
	@echo 'DEP_OPT        :' $(DEP_OPT)
	@echo 'curr path names:' $(CUR_PATH_NAMES)
	@echo 'curr path      :' $(CURDIR)

## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##
##############################################################
