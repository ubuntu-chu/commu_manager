#ifndef _INCLUEDES_H_
#define _INCLUEDES_H_

//#include    "includes-low.h"
//#include "config.h"
#include "shmem.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/LogFile.h>
#include <muduo/net/Buffer.h>
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
typedef float                   					fp32_t;
typedef double                  					fp64_t;

typedef uint8										byte;
typedef uint16 										hword;
typedef uint32 										word;
typedef uint64 										dword;



#endif

