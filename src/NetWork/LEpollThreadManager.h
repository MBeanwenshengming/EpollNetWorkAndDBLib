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

#ifndef __LINUX_EPOLL_THREAD_MANAGER_HEADER_DEFINED__
#define __LINUX_EPOLL_THREAD_MANAGER_HEADER_DEFINED__

#include "IncludeHeader.h"


class LEpollThread;
class LSession;
class LNetWorkServices;

extern unsigned int g_unEpollThreadCount;

typedef struct _Epoll_Thread_Desc
{
	volatile int nRefCount;
	LEpollThread* pEpollThread;
	_Epoll_Thread_Desc()
	{
		__sync_lock_test_and_set(&nRefCount, 0);
		pEpollThread = NULL;
	}
}t_Epoll_Thread_Desc;



class LEpollThreadManager
{
public:
	LEpollThreadManager();
	~LEpollThreadManager();
public:

	//	unEpollThreadCount EPOLL线程数量
	//	unWaitClientSizePerEpoll 每个EPOLL上监听的套接字数量, 创建epoll时使用
	//	unCloseWorkItemCount 关闭工作的最大队列数量
	bool Initialize(unsigned int unEpollThreadCount, unsigned int unWaitClientSizePerEpoll, unsigned int unCloseWorkItemCount);
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices);

public:
	bool StartAllEpollThread();
	void StopAllEpollThread();
public:
	bool PostEpollReadEvent(LSession* pSession);
public:
	bool BindEpollThread(LSession* pSession);
	void UnBindEpollThread(LSession* pSession);
	LEpollThread* GetEpollThread(unsigned int unThreadID);
	void ReleaseEpollThreadManagerResource();

protected:
	int SelectEpollThread();
private:
	t_Epoll_Thread_Desc* m_parrEpollThreadManager;
	LNetWorkServices* m_pNetWorkServices;
	unsigned int m_unEpollThreadCount;		//	epoll线程的数量
public:	// for Test
	void PrintEpollThreadStatus();
};
#endif

