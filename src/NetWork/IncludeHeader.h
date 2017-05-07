/*
The MIT License (MIT)

Copyright (c) <2010-2020> <wenshengming>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __INCLUDE_HEADER_FILE_DEFINED__
#define __INCLUDE_HEADER_FILE_DEFINED__

#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <asm/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <alsa/iatomic.h>
#include <sched.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include "./InfoRecorder/LInfoRecorderBase.h"

//	线程类型
enum E_Thread_Type
{
	E_Thread_Type_Invalid = 0,
	E_Thread_Type_Accept,
	E_Thread_Type_Epoll,
	E_Thread_Type_Recv,
	E_Thread_Type_Send,
	E_Thread_Type_Close,
	E_Thread_Type_MainLogic,
	E_Thread_Type_Connector,
	E_Thread_Type_Max,
};

#define PRINT_INFO_TO_DEBUG 1

#endif

