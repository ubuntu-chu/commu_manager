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

SRC_DIR = tinyxml parse utils reactor/muduo/net reactor/muduo/net zygote commout

CROSS_COMPILE_NAME=arm-none-linux-gnueabi-
#CROSS_COMPILE_NAME= 

.PHONY: all objs tags ctags clean distclean help show target_type info strip install 

all: 
	$(foreach d,$(SRC_DIR),cd $d;make CROSS_COMPILE=$(CROSS_COMPILE_NAME);cd ..)
#	for d in $(SRC_DIR); do \
#		cd $$(d); make CROSS_COMPILE=$(CROSS_COMPILE_NAME); cd ..\
#	done \


clean:

distclean: 



strip:

install:

## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##
##############################################################
