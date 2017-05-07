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

#ifndef __EPOLL_THREAD_HEADER_DEFINED__
#define __EPOLL_THREAD_HEADER_DEFINED__

#include "LThreadBase.h"
#include "LRecvThread.h"
#include "LNetWorkServices.h"
#include "IncludeHeader.h"
#include "LSendThread.h"

extern size_t g_sEpollWorkItemCount;
extern unsigned int g_unRecvThreadCount;
extern unsigned int g_unWaitClientPerEpoll;

class LEpollThread :
	public LThreadBase
{
public:
	LEpollThread(void);
	virtual ~LEpollThread(void);
public:
	//	unWaitClientSizePerEpoll 每个EPOLL上监听的套接字数量, 创建epoll时使用
	bool Initialize(unsigned int unWaitClientSizePerEpoll, unsigned int unCloseWorkItemCount);
public:
	virtual int ThreadDoing(void* pParam);
	int GetEpollHandle();

	void ReleaseEpollThreadResource();

	int m_nThreadID;
public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices);


private:
	int m_nEpollThreadHandle;
	struct epoll_event* m_pEvents;
	int m_nMaxEvents;
	LNetWorkServices* m_pNetWorkServices;

	void PrintBufferInfo();
	time_t m_tLastPrintBufferInfo;

public:
	int GetFPS()
	{
		return m_nFPS;
	}
	int GetMinFPS()
	{
		return m_nMinFPS;
	}
	int GetMaxFPS()
	{
		return m_nMaxFPS;
	}
private:
	volatile int m_nFPS;
	int m_nMinFPS;
	int m_nMaxFPS;
	int m_nCurrentSecondFPS;
	time_t m_tLastSecond;
public:
	//	加入一个需要处理的即将关闭的连接
	void AddWillCloseSessionInEpollThread(LSession* pSession);
	//	取一个出来处理
	bool GetOneWillCloseSessionInEpollThread(LSession** pSession);

	//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
	void ProcessCloseSessionInEpollThread(LSession* pSession);
private:
	LFixLenCircleBuf m_FixBufWillCloseSessionToProcessInEpollThread;

};
#endif


