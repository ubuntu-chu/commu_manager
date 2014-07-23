#ifndef _INCLUEDES_H_
#define _INCLUEDES_H_

//#include    "includes-low.h"
//#include "config.h"
#include "shmem.h"
#include <list.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>     /*Unix标准函数定义*/
#include <sys/types.h>  /**/
#include <sys/stat.h>   /**/
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX终端控制定义*/
#include <time.h>
#include <endian.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/LogFile.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Channel.h>
#include <muduo/base/ProcessInfo.h>
#include <muduo/net/EventLoopThread.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <tinyxml/tinyxml.h>

#include <string>
#include <vector>
#include <map>


//base type define
typedef unsigned char 							uint8;
typedef signed char 								int8;
typedef unsigned short int 						uint16;
typedef short signed int 							int16;
typedef unsigned int 								uint32;
typedef int 										int32;
typedef unsigned long long int 				    uint64;
typedef long long signed int    					int64;
typedef unsigned char 							uint8_t;
typedef signed char 								int8_t;
typedef unsigned short int 						uint16_t;
typedef short signed int 							int16_t;
typedef unsigned int 								uint32_t;
typedef int 										int32_t;
typedef unsigned long long int 				    uint64_t;
typedef long long signed int    					int64_t;

typedef float                   					fp32_t;
typedef double                  					fp64_t;

typedef uint8										byte;
typedef uint16 										hword;
typedef uint32 										word;
typedef uint64 										dword;

typedef int                                        portBASE_TYPE;
typedef unsigned int                              portuBASE_TYPE;



#endif

