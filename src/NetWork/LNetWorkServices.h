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

#ifndef __LINUX_NET_WORK_SERVICES_HEADER_DEFINED__
#define __LINUX_NET_WORK_SERVICES_HEADER_DEFINED__

#include "LRecvThreadManager.h"
#include "LSendThreadManager.h"
#include "LEpollThreadManager.h"
#include "LAcceptThread.h"
#include "LSessionManager.h"

#include "LFixLenCircleBuf.h"
#include "LPacketPoolManager.h"
#include "LRecvThread.h"
#include "LCloseSocketThread.h" 
#include "LVarLenCircleBuf.h"

typedef struct _Session_Accepted
{
	char szIp[16];
	unsigned short usPort;
	int nRecvThreadID;
	int nSendThreadID;
	unsigned short usExtDataLen;
	char szExtData[8 * 1024];
	_Session_Accepted()
	{
		memset(szIp, 		0, sizeof(szIp));
		memset(szExtData, 0, sizeof(szExtData));
		usPort 			= 0;
		nRecvThreadID 	= -1;
		nSendThreadID 	= -1;
		usExtDataLen  	= 0;

	}
}t_Session_Accepted;

class LServerBaseNetWork;

typedef struct _NetWorkServices_Params_Base
{
	char* pListenIP;					//	 监听IP
	unsigned short	usListenPort;		// 监听端口
	unsigned int	unListenListSize;	// 监听套接字上允许的排队最大数量
	unsigned int 	unAcceptThreadToMainThreadRecvedPacketPoolLen;	//	配置文件中AcceptRecvedPacketPoolLen的值,accept线程提交的接受连接的数据包的缓存大小，主线程中有个环形缓存区,用来存放accept线程提交的接受到的连接，单位：字节
}t_NetWorkServices_Params_Base;

typedef struct _NetWorkServices_Params_Session
{
	unsigned int unMaxSessionNum;						//	最大接收连接数量
	unsigned short usKickOutSessionTime;			//	踢出不通信连接的超时时间，默认为30秒
	unsigned int unMaxPacketLen;						// 接收的最大数据包，接收缓存会设置为unMaxPacketLen + sizeof(int)
	unsigned int unSendBufLen;							//	发送缓存的大小
	unsigned int unCloseWorkItemCount;				//	主线程，recv线程， send线程，epoll线程处理关闭连接的缓存区长度，为unMaxSessionNum + 100
}t_NetWorkServices_Params_Session;

typedef struct _NetWorkServices_Params_Recv_Thread
{
	unsigned short usThreadCount;						//	接收线程的数量
	unsigned int unRecvWorkItemCount;				// epollin事件环形队列的最大队列数，大于该数量，那么EPOLLIN事件无法放入接收线程处理tNetWorkServicesParams
	unsigned int unRecvThreadToMainThreadRecvedPacketPoolLen; //配置文件RecvThreadPacketPoolLen的值，recv线程提交到主线程的获取到的数据包，每个recv线程一个环形缓存区，单位：字节
}t_NetWorkServices_Params_Recv_Thread;

typedef struct _NetWorkServices_Params_Send_Thread
{
	unsigned int unSendThreadCount;						//	发送线程的数量
	unsigned int unSendWorkItemCount;					//	发送队列的长度，这里描述了需要发送的连接和数据包
	unsigned int unEpollOutEventMaxCount;				//	发送线程接收的EPOLLOUT事件的最大数量
}t_NetWorkServices_Params_Send_Thread;

typedef struct _NetWorkServices_Params_Epoll_Thread
{
	unsigned int unEpollThreadCount;					//	EPOLL线程数量
	unsigned int unWaitClientSizePerEpoll; 		//	每个EPOLL上监听的套接字数量, 创建epoll时使用
}t_NetWorkServices_Params_Epoll_Thread;

typedef struct _NetWorkServices_Params_Close_Thread
{
	unsigned int unCloseWorkItemCount;					//	最大可以提交的关闭事件数量
	unsigned int unCloseThreadToMainThreadRecvedPacketPoolLen;	//	配置文件CloseThreadPacketPoolLen的值，close线程提交到主线程的要断开的连接的信息，主线程环形缓存区存在数据，单位：字节
}t_NetWorkServices_Params_Close_Thread;


typedef struct _NetWorkServices_Params
{ 
	t_NetWorkServices_Params_Base nwspBase;
	t_NetWorkServices_Params_Session nwspSession;
	t_NetWorkServices_Params_Recv_Thread nwspRecvThread;
	t_NetWorkServices_Params_Send_Thread nwspSendThread;
	t_NetWorkServices_Params_Epoll_Thread nwspEpollThread;
	t_NetWorkServices_Params_Close_Thread nwspCloseThread;
}t_NetWorkServices_Params;

class LNetWorkServices
{
public:
	LNetWorkServices();
	~LNetWorkServices();

public:
	LRecvThreadManager& GetRecvThreadManager();
	LSendThreadManager& GetSendThreadManager();
	LEpollThreadManager& GetEpollThreadManager();
	LAcceptThread& GetAcceptThread();
	LMasterSessionManager& GetSessionManager();
	LCloseSocketThread& GetCloseSocketThread();

private:
	LRecvThreadManager m_RecvThreadManager;
	LSendThreadManager m_SendThreadManager;
	LEpollThreadManager m_EpollThreadManager;
	LAcceptThread m_AcceptThread;
	LMasterSessionManager m_SessionManager;
	LCloseSocketThread m_CloseSocketThread;

private:
	void CheckMachineIsLittleEndian();
public:
	bool Initialize(t_NetWorkServices_Params& nwsp, bool bInitialAccept = true);
	bool Start();
	void Stop();

	//	acceptThread接受连接投递连接数据包，recvThread投递接收的数据包给主线程，closeThread投递断线包给主线程
public:
	//	初始化各个线程接收数据包队列
	bool InitializeFixLenCircleBufOfRecvedPacket(unsigned int unMaxAcceptThreadItemCount, unsigned int unMaxRecvThreadItemCount, unsigned int unMaxCloseThreadItemCount);
	//	加入接收的数据包
	bool AddRecvedPacketToMainLogicVarLenCircleBuf(E_Thread_Type eThreadType, unsigned int unThreadID, char* pPacketData, unsigned int unPacketDataLen, uint64_t u64SessionID = 0);
	//	for main Logic
public:
	//	unMaxProcessRecvedPakcetNumPerQueue 每个接收队列处理最大数量
	int ProcessRecvedPacket(unsigned int unMaxProcessRecvedPakcetNumPerQueue);
	int GetProcessedCountThisRound();
private:
	int ProcessRecvedPacketFromAcceptThread(LVarLenCircleBuf* pVarLenCircleBuf, unsigned int unProcessedNum);
	int ProcessRecvedPacketFromRecvThread(LVarLenCircleBuf* pVarLenCircleBuf, unsigned int unProcessedNum);
	int ProcessRecvedPacketFromCloseThread(LVarLenCircleBuf* pVarLenCircleBuf, unsigned int unProcessedNum);
private:
	LVarLenCircleBuf* m_pArrayAcceptThreadRecvedPacket;
	LVarLenCircleBuf* m_pArrayRecvThreadRecvedPacket;
	LVarLenCircleBuf* m_pArrayCloseThreadRecvedPacket;
	int m_nProcessedThisRound;
	//	获取Accept数据包
	char m_szTempAcceptedSessionInfo[MAX_DATA_BUF_LEN_TO_WRITE_ACCEPTED_SESSION_INFO];
	unsigned int m_unTempAcceptedSessionInfoLen;

	//	获取客户端发送上来的数据包
	char m_szTempPacketData[MAX_TEMP_PACKET_BUF_LEN];			//	临时存放当前的数据包
	unsigned int m_unTempPacketDataLen;

	char m_szTempClosePacketData[32];								//	获取断开连接数据包
	unsigned int m_unTempClosePacketDataLen;
public:
	bool SendPacket(uint64_t u64SessionID, char* pData, unsigned int unDataLen);

private:
	unsigned int m_unSendThreadCount;
	unsigned int m_unAcceptThreadCount;
	unsigned int m_unRecvThreadCount;
	unsigned int m_unCloseThreadCount;
	unsigned int m_unEpollThreadCount;


public:
	//	删除一段时间内没有通讯的死连接
	void KickOutIdleSession();
	bool GetSessionIPAndPort(unsigned long long u64SessionID, char* pszBuf, unsigned short usbufLen);
public:	// For Close Socket
	void KickOutSession(uint64_t u64SessionID);
public:	//	释放占用的资源
	void ReleaseNetWorkServicesResource();





public: 
	void GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort);


public:
	//	主线程必须调用这个函数，来这却处理关闭连接
	int ProcessWillCloseSessionInMainLogic();
	void AddWillCloseSessionInMainLogic(LSession* pSession);
	bool GetOneWillCloseSessionInMainLogic(LSession** pSession);
	void ProcessKickOutSession(uint64_t u64SessionID);
private:
	LFixLenCircleBuf m_FixCircleBufWillCloseSessionInMainLogic;


public:
	void SetServerBaseNetWork(LServerBaseNetWork* pServerBaseNetWork);
private:
	LServerBaseNetWork* m_pServerBaseNetWork;

public:
	bool AddFirstRecvOfSession(uint64_t u64SessionID);
	//	日志系统
public:
	void SetLogTypeAndLevel(unsigned int unLogType, unsigned int unLevel);
	void GetLogTypeAndLevel(unsigned int& unLogType, unsigned int& unLevel);
	void SetInfoRecorderBase(LInfoRecorderBase* pInfoRecorderBase);
	void InfoRecorder(unsigned int unLogType, unsigned int unLevel, unsigned int unLogSubType, unsigned int unThreadID, const char* pszFileName, const char* pszFunctionName, int nLine, const char* pszFmt, ...);
private:
	LInfoRecorderBase* m_pInfoRecorderBase;

public:
	void PrintManagerInfos();
private:
	void PrintBufferInfos();
	time_t m_tLastPrintInfosTime;
};
#endif

