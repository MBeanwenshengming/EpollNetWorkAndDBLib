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

#ifndef __LINUX_CLOSE_THREAD_HEADER_INCLUDED_DEFINED__
#define __LINUX_CLOSE_THREAD_HEADER_INCLUDED_DEFINED__

#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"
#include "LPacketPoolManager.h"

class LNetWorkServices;
class LSession;

typedef struct _Client_Need_To_Close
{
	uint64_t u64SessionID;
}t_Client_Need_To_Close;

class LCloseSocketThread : public LThreadBase
{
public:
	LCloseSocketThread();
	~LCloseSocketThread();
public:
	//	un...Count, 线程数量
	//	unMaxWorkItemForClose 每个线程提交的最大的关闭请求数量
	bool Initialize(int nThreadID, unsigned int unAcceptThreadCount, unsigned int unEpollThreadCount, unsigned int unRecvThreadCount, unsigned int unSendThreadCount,
						unsigned int unMainLogicThreadCount, unsigned int unMaxWorkItemForClose);
	bool AppendToClose(E_Thread_Type eThreadType, unsigned int unThreadID, t_Client_Need_To_Close ClientToClose);
public: 
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop(); 
public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices)
	{
		m_pNetWorkServices = pNetWorkServices;
	}

private:
	//	处理在队列中的需要断开的连接，返回处理的数量
	int ProcessDisconnectInFixLenCircle(LFixLenCircleBuf* pFixLenCircle, int nMaxProcessCount);
	bool InitializeFixLenCircleArray(LFixLenCircleBuf* pArrayFixLenCircleBuf, unsigned int unArraySize, unsigned int unItemSize, unsigned int unFixLenCircleItemCount);
	bool ProcessDisconnect(LSession* pSession);
private:
	unsigned int m_unAcceptThreadCount;
	unsigned int m_unEpollThreadCount;
	unsigned int m_unRecvThreadCount;
	unsigned int m_unSendThreadCount;
	unsigned int m_unMainLogicThreadCount;

	LFixLenCircleBuf* m_pArrayBufSessionNeedToCloseOfAcceptThread;
	LFixLenCircleBuf* m_pArrayBufSessionNeedToCloseOfEpollThread;
	LFixLenCircleBuf* m_pArrayBufSessionNeedToCloseOfRecvThread;
	LFixLenCircleBuf* m_pArrayBufSessionNeedToCloseOfSendThread;
	LFixLenCircleBuf* m_pArrayBufSessionNeedToCloseOfMainLogicThread;

	LNetWorkServices* m_pNetWorkServices;

	int m_nThreadID;
public:
	void StopCloseSocketThread();
		
	void ReleaseCloseSocketThreadResource();

private:
	void PrintBufferInfos();
	time_t m_tLastPrintBufferTime;
};

#endif

