#ifndef _INCLUEDES_H_
#define _INCLUEDES_H_

#include    "includes-low.h"

//#define		__LINUX_OS__
#ifdef	__LINUX_OS__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#else


#endif

#include 	 "../api/utils.h"
#include 	 "../bsp/bsp_startup.h"
#include 	 "../bsp/hal/hal.h"
#include    "../bsp/cpu/cpu_tick.h"
#include    "../bsp/cpu/cpu_gpio.h"
#include    "../bsp/cpu/cpu_interrupt.h"
#include    "../bsp/cpu/cpu_uart.h"
#include    "../bsp/cpu/cpu_spi.h"
#include    "../bsp/cpu/cpu_iic.h"


#endif

