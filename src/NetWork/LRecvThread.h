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

#ifndef __RECV_THREAD_HEADER_DEFINED__
#define __RECV_THREAD_HEADER_DEFINED__
#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"

class LNetWorkServices;

typedef struct _Recv_WorkItem
{
	uint64_t u64SessionID;		// 需要接受数据的SessionID
}t_Recv_WorkItem;


#define 	MAX_TEMP_PACKET_BUF_LEN   (256 * 1024)

class LSession;

class LRecvThread :
	public LThreadBase
{
public:
	LRecvThread(void);
	virtual ~LRecvThread(void);
public:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop(); 
public:

	//	unEpollThreadCount epollThread数量
	//	unWorItemCountperFixLenCircleLen 每个epollThread可以提交的任务最大工作队列长度
	//	unWorkItemCountOfCloseSessionFixLenCircleBuf	//	连接关闭工作队列长度
	bool Initialize(unsigned int unEpollThreadCount, unsigned int unWorItemCountperFixLenCircleBuf, unsigned int unWorkItemCountOfCloseSessionFixLenCircleBuf);

public:			//	EpollThread发送过来的需求调用Recv的连接，即要进行的工作
	bool InitializeRecvwordOfFixLenCircleBuf(unsigned int unEpollThreadCount, unsigned int unMaxWorkItemCount);
	bool AddRecvworkToRecvthread(unsigned int unEpollThreadID, uint64_t u64SessionID);
	//	添加一个Session去关闭线程关闭
	void AddSessionToCloseThreadToClose(LSession* pSession);
protected:
	//	返回处理的数量
	int ProcessRecvworkFromEpollThread(unsigned int unDoWorkCountPerFixLenCircleBuf);
	int ProcessRecvSystemCall(uint64_t u64SessionID, bool bIsFirstRecv);
private:
	//	每个EpollThread一个LFixLenCircleBuf
	LFixLenCircleBuf* m_pArrayRecvWorkOfEpollthreadToRecvthread;
	//	EpollThread数量
	unsigned int m_unEpollThreadCount;
public:
	int SetTempPacketData(LSession* pSession, char* pData, unsigned int unDataLen);
	char* GetTempPacketDataBuf();
	unsigned int GetTempPacketDataLen();
private:
	char m_szTempPacketData[MAX_TEMP_PACKET_BUF_LEN];			//	临时存放当前的数据包
	unsigned int m_unPacketDataLen;
public:
	void SetNetServices(LNetWorkServices* pNetWorkServices);
	LNetWorkServices* GetNetServices()
	{
		return m_pNetWorkServices;
	}
	int m_nPacketRecved;
	int m_nThreadID;
private:
	LNetWorkServices* m_pNetWorkServices;

public:		// For Test
	void PrintRecvThreadStatus();
private:
	time_t m_tLastWriteBufDescTime;

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
	void ReleaseRecvThreadResource();

	//	即将关闭的连接，需要处理这个消息，设置已经处理为true
public:
	//	加入一个需要处理的即将关闭的连接
	void AddWillCloseSession(LSession* pSession);
	//	取一个出来处理
	bool GetOneWillCloseSession(LSession** pSession);

	//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
	void ProcessRecvDataErrorToCloseSession(LSession* pSession);
private:
	LFixLenCircleBuf m_FixBufWillCloseSessionToProcess;

	//	问题
	//	主线程接受连接之后，第一次将socket绑定到epoll的Handle上时（即主线程调用m_EpollThreadManager.PostEpollReadEvent(pSession)），
	//	Epoll线程会立即执行，并且Recv线程也会立即执行，当recv完成，调用AddToEpollWaitThread函数中的epoll_ctl，op设置为EPOLL_CTL_MOD，返回错误，该socket为在Epoll上注册，
	//	目前观察下来，是因为m_EpollThreadManager.PostEpollReadEvent中的epoll_ctl函数还没有返回，而Recv线程的epoll_ctl已经调用
	//	解决办法
	//	1.主线程投递第一个recv调用，即发送一个RecvWork给Recv线程，让recv执行完成之后，调用epoll_ctl的EPOLL_CTL_ADD，注册上socket
	//	2.另一个解决办法是，当recv线程中的AddToEpollWaitThread函数中的epoll_ctl返回未注册时，直接使用epoll_ctl的EPOLL_CTL_ADD注册，遮样的话，有两个epoll_ctl的EPOLL_CTL_ADD返回，不知道系统如何实现的
	//	保险起见采用第1中方法
public:
	bool AddFirstRecvOfSession(uint64_t u64SessionID);
	void ProcessFirstRecvOfSession();
private:
	LFixLenCircleBuf m_FixBufFirstRecvOfSession;
};
#endif

