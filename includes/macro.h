#ifndef    _MACRO_H_
#define    _MACRO_H_

#include    "types.h"
#include    "config.h"

#if BIG_ENDIAN == 0

#define LD_WORD(ptr)        (WORD)(((WORD)*(BYTE_PTR)((ptr)+1)<<8)|(WORD)*(BYTE_PTR)(ptr))

#define LD_DWORD(ptr)       (DWORD)(((DWORD)*(BYTE_PTR)((ptr)+3)<<24)   |   \
                            ((DWORD)*(BYTE_PTR)((ptr)+2)<<16)           |   \
                            ((WORD)*(BYTE_PTR)((ptr)+1)<<8)             |   \
                            *(BYTE_PTR)(ptr))

#define ST_WORD(ptr,val)    *(BYTE_PTR)(ptr)=(BYTE)(val);                   \
                            *(BYTE_PTR)((ptr)+1)=(BYTE)((WORD)(val)>>8)

#define ST_DWORD(ptr,val)   *(BYTE_PTR)(ptr)=(BYTE)(val);                   \
                            *(BYTE_PTR)((ptr)+1)=(BYTE)((WORD)(val)>>8);    \
                            *(BYTE_PTR)((ptr)+2)=(BYTE)((DWORD)(val)>>16);  \
                            *(BYTE_PTR)((ptr)+3)=(BYTE)((DWORD)(val)>>24)
#else
#define LD_WORD(ptr)        (WORD)(((WORD)*(BYTE_PTR)((ptr)+0)<<8)|(WORD)*(BYTE_PTR)((ptr)+1))

#define LD_DWORD(ptr)       (DWORD)(((DWORD)*(BYTE_PTR)((ptr)+0)<<24)   |   \
                            ((DWORD)*(BYTE_PTR)((ptr)+1)<<16)           |   \
                            ((WORD)*(BYTE_PTR)((ptr)+2)<<8)             |   \
                            *(BYTE_PTR)((ptr)+3))

#define ST_WORD(ptr,val)    *(BYTE_PTR)((ptr)+0)=(BYTE)((WORD)(val)>>8);    \
                            *(BYTE_PTR)((ptr)+1)=(BYTE)(val)

#define ST_DWORD(ptr,val)   *(BYTE_PTR)((ptr)+0)=(BYTE)((DWORD)(val)>>24);  \
                            *(BYTE_PTR)((ptr)+1)=(BYTE)((DWORD)(val)>>16);  \
                            *(BYTE_PTR)((ptr)+2)=(BYTE)((WORD)(val)>>8);    \
                            *(BYTE_PTR)((ptr)+3)=(BYTE)(val)


#endif

//------------------------------------------------------------------------------
#define   		 BIT(n)											 (1UL << n)
#define         SBI(reg, bit)                                 	 ((reg) |= BIT(bit))
#define         CBI(reg, bit)                                 	 ((reg) &= ~BIT(bit))
#define         XBI(reg, bit)                                 	 ((reg) ^= BIT(bit))
#define         BIT_IS_SET(reg, bit)                             (!!((reg) & BIT(bit)))
#define         BIT_IS_CLEAR(reg, bit)                           (!BIT_IS_SET(reg, bit))

//------------------------------------------------------------------------------
#define         XBYTE(addr)                                     (*(volatile uint8 *)(addr))
#define         XHWORD(addr)                                    (*(volatile uint16 *)(addr))
#define         XWORD(addr)                                     (*(volatile uint32 *)(addr))
#define         XDWORD(addr)                                    (*(volatile uint64 *)(addr))

/* register access macros */
#define 		 in16(var,l,h)  								 (var = ((uint16)(l)) | (((uint16)(h)) << 8))
#define 		 out16(l,h,val) 								 do {l = (uint8)(val); h = (uint8)(((uint16)val) >> 8);}while (0)

//------------------------------------------------------------------------------
#define         MIN(a,b)                                        ((a)<(b)?(a):(b))
#define         MAX(a,b)                                        ((a)<(b)?(b):(a))
#define         C2N(c)                                       	 (c - '0')
#define         N2C(n)                                          (n + '0')

//------------------------------------------------------------------------------
#define         OFFSET(Struct, Field) 				            ((unsigned int)(unsigned char*)&(((Struct *)0)->Field))
#define 		 ARRAY_SIZE(arr) 								(sizeof(arr) / sizeof((arr)[0]))

#define         ALIGN_UP(x, a)                                  (((x)+(a))&(~a))
#define         ALIGN_DOWN(x, a)                                (((x)&(~a))

#define         ROUND_UP(x, y)                                  ((((x) + ((y) - 1)) / y) * y)
#define         ROUND_DOWN(x, y)                                ((x) - ((x) % (y)))

#define         DIV_ROUND_UP(n,d)                               (((n) + (d) - 1) / (d))
#define         DIV_ROUND_DOWN(n,d)                             (((n) / (d))

//------------------------------------------------------------------------------
#define         IN_RANGES(x, start, end)                        (((x) >= (start)) && ((x) <= (end)))

//------------------------------------------------------------------------------
#if 	0
#define   		 BIT0 						   					(1UL << 0)
#define   		 BIT1 						   					(1UL << 1)
#define   		 BIT2 						   					(1UL << 2)
#define   		 BIT3 						   					(1UL << 3)
#define   		 BIT4 						   					(1UL << 4)
#define   		 BIT5 						   					(1UL << 5)
#define   		 BIT6 						   					(1UL << 6)
#define   		 BIT7 						   					(1UL << 7)
#endif
#if 	1
#define   		 BIT8 						   					(1UL << 8)
#define   		 BIT9 						   					(1UL << 9)
#define   		 BIT10 						   					(1UL << 10)
#define   		 BIT11 						   					(1UL << 11)
#define   		 BIT12 						   					(1UL << 12)
#define   		 BIT13 						   					(1UL << 13)
#define   		 BIT14 						   					(1UL << 14)
#define   		 BIT15 						   					(1UL << 15)
#endif
#if 	1
#define   		 BIT16 						   					(1UL << 16)
#define   		 BIT17 						   					(1UL << 17)
#define   		 BIT18 						   					(1UL << 18)
#define   		 BIT19 						   					(1UL << 19)
#define   		 BIT20 						   					(1UL << 20)
#define   		 BIT21 						   					(1UL << 21)
#define   		 BIT22 						   					(1UL << 22)
#define   		 BIT23 						   					(1UL << 23)
#endif
#if 	1
#define   		 BIT24 						   					(1UL << 24)
#define   		 BIT25 						   					(1UL << 25)
#define   		 BIT26 						   					(1UL << 26)
#define   		 BIT27 						   					(1UL << 27)
#define   		 BIT28 						   					(1UL << 28)
#define   		 BIT29 						   					(1UL << 29)
#define   		 BIT30 						   					(1UL << 30)
#define   		 BIT31 						   					(1UL << 31)
#endif

#ifdef LOGGER
#define ASSERT(EX)                                                         \
if (!(EX))                                                                 \
{                                                                          \
    volatile char dummy = 0;                                              \
	LOG_FATAL << "(%s) assert failed !" << #EX;							   \
    while (dummy == 0);                                                   \
}
#else

#define ASSERT(EX)                                                         \
if (!(EX))                                                                 \
{                                                                          \
    volatile char dummy = 0;                                              \
    while (dummy == 0);                                                    \
}
#endif


#include <stdarg.h>
#define SECTION(x)                      __attribute__((section(x)))
#define UNUSED                          __attribute__((unused))
#define USED                            __attribute__((used))
#define ALIGN(n)                        __attribute__((aligned(n)))





/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
