#ifndef    _TYPES_H_
#define    _TYPES_H_

//base type define
typedef unsigned char 							uint8_t;
typedef unsigned char 							uint8;
typedef signed char 								int8_t;
typedef signed char 								int8;
typedef unsigned short int 						uint16_t;
typedef unsigned short int 						uint16;
typedef short signed int 							int16_t;
typedef short signed int 							int16;
typedef unsigned int 								uint32_t;
typedef unsigned int 								uint32;
typedef signed int 								int32_t;
typedef int 										int32;
typedef unsigned long long int 				    uint64;
#if 0
typedef unsigned long long int 					uint64_t;
typedef long long signed int   					int64_t;
#endif
typedef long long signed int    					int64;
typedef float                   					fp32_t;
typedef double                  					fp64_t;

typedef unsigned int           					bool_t;  	/* Boolean type */

typedef uint8										byte;
typedef uint16 										hword;
typedef uint32 										word;
typedef uint64 										dword;

//最适合cpu类型定义
typedef int 										portBASE_TYPE;
typedef unsigned int 								portuBASE_TYPE;
//中断寄存器类型定义
typedef unsigned int 								portCPSR_TYPE;
typedef uint16                     		        portSIZE_TYPE;
typedef int16                     		            portSSIZE_TYPE;
typedef portSIZE_TYPE                              portOFFSET_TYPE;
typedef portBASE_TYPE                              sv_err_t;
typedef portCPSR_TYPE								sv_base_t;

/* error code definitions */
#define SV_EOK                          0               /**< There is no error */
#define SV_ERROR                        1               /**< A generic error happens */
#define SV_ETIMEOUT                     2               /**< Timed out */
#define SV_EFULL                        3               /**< The resource is full */
#define SV_EEMPTY                       4               /**< The resource is empty */
#define SV_ENOMEM                       5               /**< No memory */
#define SV_ENOSYS                       6               /**< No system */
#define SV_EBUSY                        7               /**< Busy */
#define SV_EIO                          8               /**< IO error */

//时间类型定义
typedef unsigned long long int 					time_t;
typedef uint32    				                    tick_t;
//typedef int 										size_t;
/* maximum value of base type */
//#define UINT8_MAX                    				0xff            /**< Maxium number of UINT8 */
//#define UINT16_MAX                   				0xffff          /**< Maxium number of UINT16 */
//#define UINT32_MAX                   				0xffffffff      /**< Maxium number of UINT32 */
#define TICK_MAX                     				((tick_t)-1)   	/**< Maxium number of tick */



//intptr_t ,uintptr_t : 表示当前平台下能够安全地对指针进行转型的整型变量
typedef unsigned int      						uintptr_t;
typedef int               							intptr_t;

//application type define
typedef uint32_t									dev_adr_t;
typedef uint32_t									sen_adr_t;
typedef char        								string_char;  	/* Signed    8 bit quantity  */


//sig atomic  当把变量声明为该类型会保证该变量在使用或赋值时, 无论是在32位还是64位的机器上都能保证操作是原子的, 它会根据机器的类型自动适应
typedef uint32_t									sig_atomic;

#define WORD                						uint16_t
#define DWORD               						uint32_t
#define BYTE                						uint8_t
#define BYTE_PTR            						uint8_t*

typedef char 										timer_handle_type;
typedef char 										event_handle_type;
typedef void (fp_void_pvoid)(void *);



#ifndef    CLOSE
    #define    CLOSE                               ('0')
#endif

#ifndef    OPEN
    #define    OPEN                                ('1')
#endif

#ifndef	   NULL
	#define    NULL                                ((portuBASE_TYPE)0)
#endif

#ifndef	   TRUE
	#define    TRUE                                ((portuBASE_TYPE)1)
	#define    true                                ((portuBASE_TYPE)1)
#endif

#ifndef	   FALSE
	#define    FALSE                               ((portuBASE_TYPE)0)
	#define    false                               ((portuBASE_TYPE)0)
#endif

#ifndef ZERO
	#define ZERO            						((portuBASE_TYPE)0)
#endif

#ifndef SUCCESS
	#define SUCCESS         						((portuBASE_TYPE)1)
#endif

#ifndef FAILE
	#define FAILE           						((portuBASE_TYPE)0)
#endif

#ifndef DEV_ONLINE
	#define DEV_ONLINE								((portuBASE_TYPE)1)
#endif

#ifndef DEV_OFFLINE
	#define DEV_OFFLINE 							((portuBASE_TYPE)0)
#endif

/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */
struct timeval {
	time_t	tv_sec;		/* seconds */
	time_t	tv_usec;	/* and microseconds */
};

/*
 * Structure defined by POSIX.1b to be like a timeval.
 */
struct timespec {
	time_t	tv_sec;		/* seconds */
	time_t	tv_nsec;	/* and nanoseconds */
};

struct timezone {
  int tz_minuteswest;	/* minutes west of Greenwich */
  int tz_dsttime;	/* type of dst correction */
};

struct tm {
  int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
  int tm_min;			/* Minutes.	[0-59] */
  int tm_hour;			/* Hours.	[0-23] */
  int tm_mday;			/* Day.		[1-31] */
  int tm_mon;			/* Month.	[0-11] */
  int tm_year;			/* Year - 1900. */
  int tm_wday;			/* Day of week.	[0-6] */
  int tm_yday;			/* Days in year.[0-365]	*/
  int tm_isdst;			/* DST.		[-1/0/1]*/

  long int tm_gmtoff;		/* Seconds east of UTC.  */
  const char *tm_zone;		/* Timezone abbreviation.  */
};


#ifdef __cplusplus 

class noncopyable
{
   protected:
      noncopyable() {}
      ~noncopyable() {}
   private: // emphasize the following members are private
      noncopyable( const noncopyable& );
      const noncopyable& operator=( const noncopyable& );
};

class copyable
{
   public:
      copyable() {}
      ~copyable() {}
      copyable( const copyable& );
      const copyable& operator=( const copyable& );
};

template<typename To, typename From>
inline To implicit_cast(From const &f) {
  return f;
}

template<typename To, typename From>     // use like this: down_cast<T*>(foo);
inline To down_cast(From* f) {                   // so we only accept pointers
  // Ensures that To is a sub-type of From *.  This test is here only
  // for compile-time type checking, and has no overhead in an
  // optimized build at run-time, as it will be optimized away
  // completely.
  if (false) {
    implicit_cast<From*, To>(0);
  }
  return static_cast<To>(f);
}

//单例模板类
template <class T>
class singleton
{
public:
     static inline T* instance(void)
     {
    	 static T t;
    	 return &t;
     }
protected:
     singleton(void);
     ~singleton(void);
private:
     singleton(const singleton&);
     singleton & operator= (const singleton &);
};

//定义单例模板类为所要访问类的友元类  用于在instance函数中访问类的构造和析构函数
#define DEFINE_SINGLETON_CLASS(type)        friend singleton<type>;

#endif




#if 0

//Tobject:调用对象的类型，Tparam回调函数参数的类型
template<typename Tobject, typename Tparam>
class CCallbackProxy
{
	typedef void (Tobject::*CbFun)(Tparam*);
public:
	CCallbackProxy(Tobject *pInstance, CbFun pFun)
    {
        m_pInstance = pInstance; 
        pCbFun = pFun;
    }
    void operator()(Tparam* pParam)
    {
        (m_pInstance->*pCbFun)(pParam);
    }

private:	
	CbFun		pCbFun;		//回调函数指针
	Tobject*	m_pInstance;	//调用对象
};

#if 0
//设置调用对象及其回调函数
template<typename Tobject, typename Tparam>
void CCallbackProxy<Tobject, Tparam>::Set(Tobject *pInstance , CbFun pFun)
{
	m_pInstance = pInstance; 
	pCbFun = pFun;
};

//调用回调函数
template<typename Tobject, typename Tparam>
bool CCallbackProxy<Tobject, Tparam>::Exec(Tparam* pParam)
{
	(m_pInstance->*pCbFun)(pParam);
	return true;
}
#endif

#endif

/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
