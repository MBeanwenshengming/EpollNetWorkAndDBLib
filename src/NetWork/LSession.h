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

#ifndef __LINUX_SESSION_HEADER_DEFINED__
#define __LINUX_SESSION_HEADER_DEFINED__

#include "IncludeHeader.h"
#include "LSocket.h"
#include "LFixLenCircleBuf.h"

class LRecvThread;
#define MAX_PACKET_SIZE 8 * 1024
typedef struct _Epoll_Bind_Param
{
	uint64_t u64SessionID;
	int nSocket;
}t_Epoll_Bind_Param;

class LPacketBroadCast;
class LSendThread;
class LCloseSocketThread;
class LPacketSingle;
class LNetWorkServices;

class LSession : public LSocket
{
public:
	LSession();
	~LSession();
public:
	//	初始化接收和发送缓存大小
	bool InitializeRecvAndSendBuf(unsigned int unSessionRecvBufLen, unsigned int unSessionSendBufferLen);
	//	client ID sessionManager分配
	void SetSessionID(uint64_t u64SessionID);
	uint64_t GetSessionID();

	// 设置接收线程ID
	void SetRecvThreadID(int unRecvThreadID);
	int GetRecvThreadID();

	//	设置发送线程ID
	void SetSendThreadID(int unSendThreadID);
	int GetSendThreadID();

	//	设置Epoll线程ID
	void SetEpollThreadID(int unEpollThreadID);
	int GetEpollThreadID();

	//	Session重用时，需要重置Session的变量	
	void Reset();
private:
	uint64_t m_u64SessionID;
	//	绑定的发送线程的ID	
	volatile int m_nRecvThreadID;
	//	绑定的发送线程的ID
	volatile int m_nSendThreadID;
	//	绑定的Epoll线程的ID
	volatile int m_nEpollThreadID;

	//	session安全关闭相关的数据
		//	只有在接收线程，发送线程，主线程,Epoll线程退出处理的情况下，才可以回收session,重新分配给新连接上来的连接
		//	因为这个几个线程会操作session的buf，所以需要完全安全的情况下，才能重用session
public:
	void GetStopProcessInfos(int& nSendThreadStopProcessInfo, int& nRecvThreadStopProcessInfo,
			int& nMainLogicThreadStopProcessInfo, int& nEpollThreadSopProcessInfo);
	void SetSendThreadStopProcess(int nStop);
	int GetSendThreadStopProcess();
	void SetRecvThreadStopProcess(int nStop);
	int GetRecvThreadStopProcess();
	void SetMainLogicThreadStopProcess(int nStop);
	int GetMainLogicThreadStopProcess();
	void SetEpollThreadStopProcess(int nStop);
	int GetEpollThreadSopProcess();
	void SetCloseWorkSendedToCloseThread(int nSended);
	int GetCloseWorkSendedToCloseThread();
private:
	volatile int m_nSendThreadStopProcessSession;		//	发送线程停止处理Session
	volatile int m_nRecvThreadStopProcessSession;		//	接收线程停止处理Session
	volatile int m_nMainLogicThreadStopProcessSession;	//	主线程停止处理Session
	volatile int m_nEpollThreadStopProcessSession;		//	Epoll线程停止处理Session
	volatile int m_nCloseWorkSendedToCloseThread; 		//	关闭工作已经发往closethread，不用再发送了

public:
	void RecvData(LRecvThread* pRecvThread, bool bIsFirstRecv);
	void ResetRecv();
private:
	char* m_pszRecvedContent;
	unsigned int m_unMaxPacketLen;
	unsigned int m_unRecvBufLen;
	unsigned int m_unCurrentContentLen;

public:
	t_Epoll_Bind_Param* GetEpollBindParam()
	{
		return &m_EpollBindParam;
	}
	void SetEpollBindParam(uint64_t u64SessionID, int nSocket)
	{
		m_EpollBindParam.u64SessionID 	= u64SessionID;
		m_EpollBindParam.nSocket			= nSocket;
	}
	bool AddToEpollWaitThread(LRecvThread* pRecvThread, bool bIsFirstRecv);
private:
	t_Epoll_Bind_Param m_EpollBindParam;

public:			//	 发送相关
	void SetCanSend(bool bValue)
	{ 
		m_bCanSend = bValue;
	}
	bool GetCanSend()
	{
		return m_bCanSend;
	}
private:
	bool m_bCanSend;


public:
	time_t GetSessionConnecttedTime()
	{
		return m_tSessionConnectted;
	}
	void SetSessionConnecttedTime()
	{
		m_tSessionConnectted = time(NULL);
	}
	int GetLastRecvTime()
	{
		return m_nLastRecvTime;
	}
	void UpdateLastRecvTime()
	{
		time_t tNow = time(NULL);
		int nLastRecvTime = tNow - m_tSessionConnectted;
		__sync_lock_test_and_set(&m_nLastRecvTime, nLastRecvTime);
	}
	int GetLastSendTime()
	{
		return m_nLastSendTime;
	}
	void UpdateLastSendTime()
	{ 
		time_t tNow = time(NULL);
		int nLastSendTime = tNow - m_tSessionConnectted;
		__sync_lock_test_and_set(&m_nLastSendTime, nLastSendTime);
	}
	bool GetIpAndPort(char* pszBuf, unsigned short usbufLen);
private:
	time_t m_tSessionConnectted;	// 连接建立的时间
	volatile int m_nLastRecvTime;		//	最后一次接收数据的时间
	volatile int m_nLastSendTime;		//	最后一次发送数据的时间

public:
	//	释放Session占用的资源
	void ReleaseSessionResource();


public:
	bool AppendDataToSend(char* pData, unsigned int unDataLen);
	int SendDataInFixCircleSendBuf(LSendThread* pSendThread, bool bFromEpollOutEvent);
	bool GetSendBufIsFull();
private:
	int SendDataFailedProcess(LSendThread* pSendThread, int nResult);
private:
	LFixLenCircleBuf m_FixLenCircleBufForSendData;
	volatile int m_nSendBufIsFull;

#if PRINT_INFO_TO_DEBUG
public:
	int GetPacketRecved()
	{
		return m_nPacketRecved;
	}
	void AddPacketRecved(int nValue)
	{
		m_nPacketRecved += nValue;
	}
	int GetPacketProcessed()
	{
		return m_nPacketProcessed;
	}
	void AddPacketProcessed(int nValue)
	{
		m_nPacketProcessed += nValue;
	}
	int GetPacketAllSendBytes()
	{
		return m_nPacketAllSendBytes;
	}
	void AddPacketAllSendBytes(int nValue)
	{
		m_nPacketAllSendBytes += nValue;
	}
	int GetPacketAllSendedBytes()
	{
		return m_nPacketAllSendedBytes;
	}
	void AddPacketAllSendedBytes(int nValue)
	{
		m_nPacketAllSendedBytes += nValue;
	}
	void ResetStatistic()
	{
		m_nPacketRecved = 0;
		m_nPacketProcessed = 0;
		m_nPacketAllSendBytes = 0;
		m_nPacketAllSendedBytes = 0;
		m_nMaxDataLenInSendBuf 	= 0;
		m_nPacketRecvedBytes = 0;
	}
	void SetMaxDatalenInSendBuf(int nValue)
	{
		if (nValue > m_nMaxDataLenInSendBuf)
		{
			m_nMaxDataLenInSendBuf = nValue;
		}
	}
	int GetMaxDataLenInSendBuf()
	{
		return m_nMaxDataLenInSendBuf;
	}
	void AddPacketRecvedBytes(int nValue)
	{
		m_nPacketRecvedBytes += nValue;
	}
	int GetPacketRecvedBytes()
	{
		return m_nPacketRecvedBytes;
	}
private:
	int m_nPacketRecved;				//	接收线程接收的数据包个数数量
	int m_nPacketProcessed;			//	主线程处理的数据包数量
	int m_nPacketRecvedBytes;		//	接收到的字节数量
	int m_nPacketAllSendBytes;		//	主线程发送的数据量
	int m_nPacketAllSendedBytes;	//	发送线程发送的数据量
	int m_nMaxDataLenInSendBuf;	//	发送缓存中存在的最大数据大小
#endif
};

#endif

