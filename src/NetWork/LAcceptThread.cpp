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

#include "LAcceptThread.h"
#include "LSessionManager.h"
#include "LRecvThreadManager.h"
#include "LSendThreadManager.h"
#include "LEpollThreadManager.h"
#include "LNetWorkServices.h"
#include <sys/prctl.h>
#include "LCloseSocketThread.h"

extern int errno;

LAcceptThread::LAcceptThread(void)
{
	m_pNetWorkServices 	= NULL; 
	memset(m_szListenIp, 0, sizeof(m_szListenIp));
	m_usListenPort 		= 0;
	m_unThreadID			= 0;

	memset(m_szDataToWriteAcceptedSessionInfo, 0, MAX_DATA_BUF_LEN_TO_WRITE_ACCEPTED_SESSION_INFO);
	m_unWriteAcceptedSessionInfoDataLen = 0;
	m_nEpollListenHandle	= -1;
	memset(&m_eventAccpet, 0, sizeof(m_eventAccpet));
}

LAcceptThread::~LAcceptThread(void)
{
}

bool LAcceptThread::Initialize(unsigned int unThreadID, char* pListenIP, unsigned short usListenPort, unsigned int unListenListSize, unsigned int unSystemRecvBufLen, unsigned int unSystemSendBufLen, bool bInitialListen)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	if (unThreadID == 0)
	{
		return false;
	}
	m_unThreadID = unThreadID;

	if (bInitialListen)
	{
		if (pListenIP == NULL)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::Initialize", __LINE__,
							"pListenIP == NULL\n");
			return false;
		}
		if (usListenPort == 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::Initialize", __LINE__,
							"usListenPort == 0\n");
			return false;
		}
		strncpy(m_szListenIp, pListenIP, sizeof(m_szListenIp) - 1);
		m_usListenPort = usListenPort;

		if (unListenListSize == 0)
		{
			unListenListSize = 1000;
		}
		int nErrorCode = m_listenSocket.Initialized(pListenIP, usListenPort, unSystemRecvBufLen, unSystemSendBufLen);
		if (nErrorCode != 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::Initialize", __LINE__,
							"m_listenSocket.Initialized Failed, IP:%s, Port:%hu, ErrorCode:%d, SystemErrorCode:%d\n", pListenIP, usListenPort, nErrorCode, errno);
			return false;
		}
		nErrorCode = m_listenSocket.Listen(unListenListSize);
		if (nErrorCode != 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::Initialize", __LINE__,
							"m_listenSocket.Listen Failed, IP:%s, Port:%hu, ErrorCode:%d, SystemErrorCode:%d\n", pListenIP, usListenPort, nErrorCode, errno);
			return false;
		}
		m_nEpollListenHandle = epoll_create(1);
		if (m_nEpollListenHandle == -1)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::Initialize", __LINE__,
					"m_nEpollListenHandle == -1, epoll_create Failed");
			return false;
		}
	}	
	return true;
}
int LAcceptThread::ReAddListenSocketToEpollHandle()
{
		struct epoll_event epollEvent;
		memset(&epollEvent, 0, sizeof(epollEvent));

		epollEvent.events = EPOLLIN | EPOLLONESHOT;

		int nAddResult = epoll_ctl(m_nEpollListenHandle, EPOLL_CTL_MOD, m_listenSocket.GetSocket(), &epollEvent);
		if (nAddResult == -1)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ReAddListenSocketToEpollHandle", __LINE__,
																				"epoll_ctl Failed , ErrorCode:%d\n", errno);
			return -1;
		}
		return 0;
}

int LAcceptThread::ThreadDoing(void* pParam)
{
	char szThreadName[128];
	sprintf(szThreadName, "AcceptThread");
	prctl(PR_SET_NAME, szThreadName);
	if (m_pNetWorkServices == NULL)
	{
		return -1;
	}
	//	添加监听到epoll句柄上面
	struct epoll_event epollEvent;
	memset(&epollEvent, 0, sizeof(epollEvent));

	epollEvent.events = EPOLLIN | EPOLLONESHOT;

	int nAddResult = epoll_ctl(m_nEpollListenHandle, EPOLL_CTL_ADD, m_listenSocket.GetSocket(), &epollEvent);
	if (nAddResult == -1)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
																			"epoll_ctl Failed , ErrorCode:%d\n", errno);
		return -1;
	}
	while(true)
	{
		if (CheckForStop())		//	收到线程退出通知，那么退出线程
		{
			break;
		}

		int nEpollEventCount = epoll_wait(m_nEpollListenHandle, &m_eventAccpet, 1, -1);
		if (nEpollEventCount < 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
																						"epoll_wait Failed , ErrorCode:%d\n", errno);
			return -1;
		}
		else if (nEpollEventCount == 0)
		{
			if (ReAddListenSocketToEpollHandle() == -1)
			{
				return -1;
			}
			continue;
		}

		int newClient = accept(m_listenSocket.GetSocket(), NULL, NULL);
		if (newClient == -1)
		{
			if (errno == EAGAIN || errno == EINTR)		//	
			{
				struct timespec timeReq;
				timeReq.tv_sec 	= 0;
				timeReq.tv_nsec 	= 10;
				nanosleep(&timeReq, NULL);
				if (ReAddListenSocketToEpollHandle() == -1)
				{
					return -1;
				}
				continue;
			}
			else
			{
				m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
														"accept Failed, ErrorCode:%d\n", errno);
				break;		//	出错，那么退出接收线程
			}
		}

		//		设置为非阻塞套接字
		int nSetNonBlockSuccess = fcntl(newClient, F_SETFL, O_NONBLOCK);
		if (nSetNonBlockSuccess == -1)
		{
			close(newClient);
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
																	"fcntl SetNonBlock Failed , ErrorCode:%d\n", errno);
			if (ReAddListenSocketToEpollHandle() == -1)
			{
				return -1;
			}
			continue;
		}
	
		//	有连接上来
		LMasterSessionManager* pMasterSessManager = &m_pNetWorkServices->GetSessionManager();
		LSession* pSession = pMasterSessManager->AllocSession(newClient);
		if (pSession == NULL)
		{
			close(newClient);
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_WARNING, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
																				"AllocSession Failed\n");
			if (ReAddListenSocketToEpollHandle() == -1)
			{
				return -1;
			}

			continue;
		}
#if 1
		int nTempRecvBufLen = 0;
		socklen_t nTempRecvLen = sizeof(nTempRecvLen);
		if (pSession->GetSockOpt(SOL_SOCKET, SO_RCVBUF, &nTempRecvBufLen, &nTempRecvLen) != 0)
		{
		}

		int nTempSendBufLen = 0;
		socklen_t nTempSendLen = sizeof(nTempSendBufLen);
		if (pSession->GetSockOpt(SOL_SOCKET, SO_SNDBUF, &nTempSendBufLen, &nTempSendLen) != 0)
		{
		}
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
																						"SystemRecvBufLen:%d, SystemSendBufLen:%d\n", nTempRecvBufLen, nTempSendBufLen);
#endif
		LRecvThreadManager* pRecvThreadManager = &m_pNetWorkServices->GetRecvThreadManager();
		if (!pRecvThreadManager->BindRecvThread(pSession))
		{ 
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
																							"pRecvThreadManager->BindRecvThread Failed\n");
			AddWillCloseSessionToCloseThread(pSession);

			if (ReAddListenSocketToEpollHandle() == -1)
			{
				return -1;
			}

			continue;
		}

		LSendThreadManager* pSendThreadManager = &m_pNetWorkServices->GetSendThreadManager();
		if (!pSendThreadManager->BindSendThread(pSession))
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
																							"pSendThreadManager->BindSendThread Failed\n");
			AddWillCloseSessionToCloseThread(pSession);
			if (ReAddListenSocketToEpollHandle() == -1)
			{
				return -1;
			}

			continue;
		}

		LEpollThreadManager* pEpollThreadManager = &m_pNetWorkServices->GetEpollThreadManager();
		if (!pEpollThreadManager->BindEpollThread(pSession))
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
																										"pEpollThreadManager->BindEpollThread Failed\n");
			AddWillCloseSessionToCloseThread(pSession);
			if (ReAddListenSocketToEpollHandle() == -1)
			{
				return -1;
			}

			continue;
		}

		//	将必要的信息填入数据包中
		BuildAcceptedPacket(pSession);

		if (!m_pNetWorkServices->AddRecvedPacketToMainLogicVarLenCircleBuf(E_Thread_Type_Accept, m_unThreadID, m_szDataToWriteAcceptedSessionInfo, m_unWriteAcceptedSessionInfoDataLen))
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::ThreadDoing", __LINE__,
																																"m_pNetWorkServices->AddRecvedPacketToMainLogicVarLenCircleBuf\n");
			AddWillCloseSessionToCloseThread(pSession);
			if (ReAddListenSocketToEpollHandle() == -1)
			{
				return -1;
			}

			continue;
		}

		//	所有处理都成功了，那么监听下一个连接的到来
		if (ReAddListenSocketToEpollHandle() == -1)
		{
			return -1;
		}
		//	这里不再添加EPOLLIN事件，等主线程处理的AcceptRecvedPacket后，再投递EPOLLIN事件，
		//	因为如果直接投递epollin，有可能主线程AcceptRecvedPacket还没有处理，Recvhread的包已经到了主线程了，造成丢包
	}
	return 0;
}
void LAcceptThread::AddWillCloseSessionToCloseThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}
	t_Client_Need_To_Close cntc;
	cntc.u64SessionID = pSession->GetSessionID();
	LCloseSocketThread* pCloseThread = &(m_pNetWorkServices->GetCloseSocketThread());
	if (!pCloseThread->AppendToClose(E_Thread_Type_Accept, m_unThreadID, cntc))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::AddWillCloseSessionToCloseThread", __LINE__,
																																		"pCloseThread->AppendToClose Failed, SessionID:%llu\n", cntc.u64SessionID);
	}
	else
	{
		pSession->SetCloseWorkSendedToCloseThread(1);
	}
}

bool LAcceptThread::OnStart()
{
	return true;
}
void LAcceptThread::OnStop()
{
}

void LAcceptThread::SetNetWorkServices(LNetWorkServices* pNws)
{
	m_pNetWorkServices = pNws;
}

void LAcceptThread::BuildAcceptedPacket(LSession* pSession)
{
	m_unWriteAcceptedSessionInfoDataLen = 0;
	if (pSession == NULL)
	{
		return ;
	}
	uint64_t u64SessionID = pSession->GetSessionID();

	char szRemoteIPName[20]; memset(szRemoteIPName, 0, sizeof(szRemoteIPName));
	unsigned short usRemotePort = 0;
	int nRes = pSession->GetPeerName(szRemoteIPName, sizeof(szRemoteIPName) - 1, usRemotePort);
	if (nRes != 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::BuildAcceptedPacket", __LINE__,
																																				"GetPeerName Failed, SessionID:%llu, SystemErrorCode:%d\n", u64SessionID, errno);
	}
	int nRecvThreadID = pSession->GetRecvThreadID();
	int nSendThreadID = pSession->GetSendThreadID();

	memmove(m_szDataToWriteAcceptedSessionInfo + m_unWriteAcceptedSessionInfoDataLen, &u64SessionID, sizeof(uint64_t));
	m_unWriteAcceptedSessionInfoDataLen += sizeof(uint64_t);

	memmove(m_szDataToWriteAcceptedSessionInfo + m_unWriteAcceptedSessionInfoDataLen, szRemoteIPName, 20);
	m_unWriteAcceptedSessionInfoDataLen += 20;

	memmove(m_szDataToWriteAcceptedSessionInfo + m_unWriteAcceptedSessionInfoDataLen, &usRemotePort, sizeof(unsigned short));
	m_unWriteAcceptedSessionInfoDataLen += sizeof(unsigned short);

	memmove(m_szDataToWriteAcceptedSessionInfo + m_unWriteAcceptedSessionInfoDataLen, &nRecvThreadID, sizeof(int));
	m_unWriteAcceptedSessionInfoDataLen += sizeof(int);

	memmove(m_szDataToWriteAcceptedSessionInfo + m_unWriteAcceptedSessionInfoDataLen, &nSendThreadID, sizeof(int));
	m_unWriteAcceptedSessionInfoDataLen += sizeof(int);
}

//	 释放线程使用的资源
void LAcceptThread::ReleaseAcceptThreadResource()
{
}

void LAcceptThread::StopAcceptThread()
{
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		//	停止线程
		Stop();
		//	等待返回
		int nJoinRes = pthread_join(pID, NULL);
		if (nJoinRes != 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_ACCEPT_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LAcceptThread::StopAcceptThread", __LINE__,
																											"pthread_join Failed, ErrorCode:%d\n", errno);
		}
	}
} 


void LAcceptThread::GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort)
{
	if (pBuf == NULL)
	{
		return ;
	}
	strncpy(pBuf, m_szListenIp, unBufSize);
	usPort = m_usListenPort;
}
