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

#ifndef __SEND_THREAD_HEADER_DEFINED__
#define __SEND_THREAD_HEADER_DEFINED__
#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"
#include <map>
using namespace std;

class LPacketBase;
class LNetWorkServices;
class LPacketBroadCast;
class LSession;

#define SEND_THREAD_SEND_BUF_LEN 128 * 1024


typedef struct _Send_Content_Desc
{
	uint64_t u64SessionID;
}t_Send_Content_Desc;


typedef struct _Send_Epoll_Out_Event
{
	uint64_t u64SessionID;
}t_Send_Epoll_Out_Event;


class LSendThread :
	public LThreadBase
{
public:
	LSendThread(void);
	virtual ~LSendThread(void);
public:

	//	unSendWorkItemCount 发送环形缓冲池大小，主线程可以放入的发送工作数量
	//	unEpollThreadCount	EpollThread数量
	//	unEpollOutEventMaxCount  EpollOut环形缓冲池队列的大小，每个epollThread对应一个环形缓冲池
	//	unCloseWorkItemCount     CloseThread放入关闭工作队列大小
	bool Initialize(unsigned int unSendWorkItemCount, unsigned int unEpollThreadCount, unsigned int unEpollOutEventMaxCount, unsigned int unCloseWorkItemCount);

public:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop(); 

public:
	//	主线程添加一个发送操作请求
	bool AddOneSendWorkItem(uint64_t u64SessionID);
private:
	bool GetOneSendWorkItem(uint64_t& u64SessionID);
	//	处理发送工作
	int ProcessSendWork(int nMaxProcessSendWorkItem);
private:
	LFixLenCircleBuf m_SendCircleBuf;

public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices);
	LNetWorkServices* GetNetWorkServices();	
	int m_nThreadID;
private:
	LNetWorkServices* m_pNetWorkServices;


public:

	int m_nRealSendCount;
public:
	void PrintSendThreadLocalBufStatus();
private:
	time_t m_tLastWriteErrorTime; 

	int m_nFreePacketCount;
protected:
	void PrintAllBufferInfo();
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
	int m_nMaxSendCountInSendCircleBuf;
public:
	bool AddEpollOutEvent(unsigned int unEpollThreadID, uint64_t u64SessionID);
private:
	//	处理EpollOut工作
	int ProcessEpollOutEvent(unsigned int unMaxProcessCountPerFixLenCircleBuf);
	bool GetOneEpollOutEvent(LFixLenCircleBuf* pFixLenCircleBuf, uint64_t& u64SessionID);
private:
	LFixLenCircleBuf* m_pArrayEpollOutFromEpollThread;
	unsigned int m_unEpollThreadCount;

public:
	void ReleaseSendThreadResource();

public:
	//	加入一个需要处理的即将关闭的连接
	void AddWillCloseSessionInSendThread(LSession* pSession);
	//	取一个出来处理
	bool GetOneWillCloseSessionInSendThread(LSession** pSession);

	//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
	void ProcessSendDataErrorToCloseSession(LSession* pSession);
private:
	LFixLenCircleBuf m_FixCircleBufWillSessionCloseToProcessSendThread;
};
#endif


