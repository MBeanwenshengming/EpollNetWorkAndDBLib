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

#include "LEpollThread.h" 
#include "LRecvThreadManager.h"
#include "LRecvThread.h"
#include "LSessionManager.h"
#include "LSession.h" 
#include <sys/prctl.h>


LEpollThread::LEpollThread(void)
{
	m_pEvents 					= NULL;
	m_nEpollThreadHandle 	= -1;
	m_nMaxEvents 				= 0;
	m_pNetWorkServices 		= NULL;
	m_nThreadID 				= -1;
	m_tLastPrintBufferInfo	= 0;
	m_nFPS						= 0;
	m_nCurrentSecondFPS		= 0;
	m_tLastSecond				= 0;
	m_nMinFPS					= 0x0FFFFFFF;
	m_nMaxFPS					= 0;
}

LEpollThread::~LEpollThread(void)
{
}

int LEpollThread::ThreadDoing(void* pParam)
{

	char szThreadName[128];
	sprintf(szThreadName, "EPollThread_%d", m_nThreadID);
	prctl(PR_SET_NAME, szThreadName);

	m_tLastPrintBufferInfo = time(NULL);
	while (1)
	{
		if (CheckForStop())
		{
			break;
		}
#if PRINT_INFO_TO_DEBUG
		time_t tNow = time(NULL);
		if (m_tLastSecond != tNow)
		{
			__sync_lock_test_and_set(&m_nFPS, m_nCurrentSecondFPS);

			if (m_nCurrentSecondFPS != 0 && m_nCurrentSecondFPS > m_nMaxFPS)
			{
				m_nMaxFPS = m_nCurrentSecondFPS;
			}
			if (m_nCurrentSecondFPS != 0 && m_nCurrentSecondFPS < m_nMinFPS)
			{
				m_nMinFPS = m_nCurrentSecondFPS;
			}
			m_nCurrentSecondFPS = 0;
			m_tLastSecond = tNow;
		}
		else
		{
			m_nCurrentSecondFPS++;
		}

#endif
		int nWillCloseSessionProcessed = 0;
		LSession* pSession = NULL;
		while (GetOneWillCloseSessionInEpollThread(&pSession))
		{
			nWillCloseSessionProcessed++;
			if (pSession != NULL)
			{
				ProcessCloseSessionInEpollThread(pSession);
			}
			if (nWillCloseSessionProcessed > 100)
			{
				break;
			}
		}

		int nEpollEventCount = epoll_wait(m_nEpollThreadHandle, m_pEvents, m_nMaxEvents, 1);
		if (nEpollEventCount < 0)
		{
			if (errno == 4)
			{
				continue;
			}
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_EPOLL_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::ThreadDoing", __LINE__,
												"epoll_wait Failed, Thread Quit, ErrorCode:%d\n", errno);
			return -1;
		}
		else if (nEpollEventCount == 0)
		{
			continue;
		}
		else
		{
		}

		for (int nIndex = 0; nIndex < nEpollEventCount; ++nIndex)
		{
			uint64_t u64SessionID = m_pEvents[nIndex].data.u64;
			LMasterSessionManager* pSessionManager = &(m_pNetWorkServices->GetSessionManager());
			LSession* pSession = pSessionManager->FindSession(u64SessionID);
			if (pSession == NULL)
			{
				continue;
			}
			//	已经在关闭了，就不处理了
			if (pSession->GetCloseWorkSendedToCloseThread() != 0)
			{
				continue;
			}

			int nRecvThreadID = pSession->GetRecvThreadID();
			if (nRecvThreadID == -1)
			{
				//	能获取到Session,那么不可能线程ID为-1
				m_pNetWorkServices->InfoRecorder(LOG_TYPE_EPOLL_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::ThreadDoing", __LINE__,
																"unRecvThreadID == -1, SessionID:%llu\n", pSession->GetSessionID());
			}
			else
			{
				if (m_pEvents[nIndex].events & EPOLLIN)
				{
					LRecvThread* pRecvThread = m_pNetWorkServices->GetRecvThreadManager().GetRecvThread(nRecvThreadID);
					if (pRecvThread != NULL)
					{
						pRecvThread->AddRecvworkToRecvthread(m_nThreadID, pSession->GetSessionID());
					}
					else
					{
						m_pNetWorkServices->InfoRecorder(LOG_TYPE_EPOLL_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::ThreadDoing", __LINE__,
																						"pRecvThread == NULL, ThreadID:%d, SessionID:%llu\n", nRecvThreadID, pSession->GetSessionID());
					}
				}
			}

			int nSendThreadID = pSession->GetSendThreadID();
			if (nSendThreadID == -1)
			{
				m_pNetWorkServices->InfoRecorder(LOG_TYPE_EPOLL_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::ThreadDoing", __LINE__,
																				"nSendThreadID == -1, SessionID:%llu\n", pSession->GetSessionID());
			}
			else
			{
				if (m_pEvents[nIndex].events & EPOLLOUT)
				{
					LSendThread* pSendThread = m_pNetWorkServices->GetSendThreadManager().GetSendThread(nSendThreadID);
					if (pSendThread != NULL)
					{
						pSendThread->AddEpollOutEvent(m_nThreadID, pSession->GetSessionID());
					}
					else
					{
						m_pNetWorkServices->InfoRecorder(LOG_TYPE_EPOLL_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::ThreadDoing", __LINE__,
																												"pSendThread == NULL, ThreadID:%u, SessionID:%llu\n", nSendThreadID, pSession->GetSessionID());
					}
				}
			}
		}
#if PRINT_INFO_TO_DEBUG
		if (tNow - m_tLastPrintBufferInfo > 180)
		{
			PrintBufferInfo();
			m_tLastPrintBufferInfo = tNow;
		}
#endif
	}
	return 0;
}

void LEpollThread::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
{
	m_pNetWorkServices = pNetWorkServices;
}


//	unWaitClientSizePerEpoll 每个EPOLL上监听的套接字数量, 创建epoll时使用
bool LEpollThread::Initialize(unsigned int unWaitClientSizePerEpoll, unsigned int unCloseWorkItemCount)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}

	if (unWaitClientSizePerEpoll == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::Initialize", __LINE__,
				"unWaitClientSizePerEpoll == 0\n");
		return false;
	}
	m_nMaxEvents = unWaitClientSizePerEpoll + 100;
	m_pEvents = new epoll_event[m_nMaxEvents];
	if (m_pEvents == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::Initialize", __LINE__,
				"m_pEvents == NULL, MaxEvents:%d\n", m_nMaxEvents);
		return false;
	}


	if (!m_FixBufWillCloseSessionToProcessInEpollThread.Initialize(sizeof(LSession*), unCloseWorkItemCount))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::Initialize", __LINE__,
						"m_FixBufWillCloseSessionToProcessInEpollThread.Initialize Failed, CloseWorkItemCount:%u\n", unCloseWorkItemCount);
		return false;
	}

	m_nEpollThreadHandle = epoll_create(unWaitClientSizePerEpoll);
	if (m_nEpollThreadHandle == -1)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::Initialize", __LINE__,
								"epoll_create Failed, SystemErrorCode:%d, WaitClientSizePerEpoll:%u\n", errno, unWaitClientSizePerEpoll);
		return false;
	}
	return true;
}
void LEpollThread::PrintBufferInfo()
{
	m_pNetWorkServices->InfoRecorder(LOG_TYPE_EPOLL_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LEpollThread::PrintBufferInfo", __LINE__,
									"\tEpollThread Print BufferInfo, ThreadID:%d\n", m_nThreadID);
	int nCloseWorkCount = m_FixBufWillCloseSessionToProcessInEpollThread.GetCurrentExistCount();
	int nCloseMaxCount = m_FixBufWillCloseSessionToProcessInEpollThread.GetMaxItemCount();
	m_pNetWorkServices->InfoRecorder(LOG_TYPE_EPOLL_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LEpollThread::PrintBufferInfo", __LINE__,
										"\t\tEpollThreadID:%d, CloseWorkCount:%d, CloseMaxCount:%d, PercentInfo:%f\n",
										m_nThreadID, nCloseWorkCount, nCloseMaxCount, float(nCloseWorkCount) / float(nCloseMaxCount));
}
int LEpollThread::GetEpollHandle()
{
	return m_nEpollThreadHandle;
}


//	加入一个需要处理的即将关闭的连接
void LEpollThread::AddWillCloseSessionInEpollThread(LSession* pSession)
{
	E_Circle_Error eError = m_FixBufWillCloseSessionToProcessInEpollThread.AddItems((char*)&pSession, 1);
	if (eError != E_Circle_Buf_No_Error)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_EPOLL_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::AddWillCloseSessionInEpollThread", __LINE__,
										"m_FixBufWillCloseSessionToProcessInEpollThread.AddItems 1, Error:%d\n", eError);
	}
}
//	取一个出来处理
bool LEpollThread::GetOneWillCloseSessionInEpollThread(LSession** pSession)
{
	E_Circle_Error eError = m_FixBufWillCloseSessionToProcessInEpollThread.GetOneItem((char*)(&(*pSession)), sizeof(LSession*));
	if (eError != E_Circle_Buf_No_Error && eError != E_Circle_Buf_Is_Empty)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_EPOLL_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LEpollThread::GetOneWillCloseSessionInEpollThread", __LINE__,
												"m_FixBufWillCloseSessionToProcessInEpollThread.GetOneItem Failed, Error:%d\n", eError);
		return false;
	}
	if (eError == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}

//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
void LEpollThread::ProcessCloseSessionInEpollThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}

	//	设置本接收线程不再对该连接进行操作
	pSession->SetEpollThreadStopProcess(1);

	//	检查是否需要关闭连接
	int nSendThreadStopProcessInfo 		= 0;
	int nRecvThreadStopProcessInfo		= 0;
	int nMainLogicThreadStopProcessInfo	= 0;
	int nEpollThreadStopProcessInfo		= 0;
	pSession->GetStopProcessInfos(nSendThreadStopProcessInfo, nRecvThreadStopProcessInfo, nMainLogicThreadStopProcessInfo, nEpollThreadStopProcessInfo);
	if (nSendThreadStopProcessInfo == 1 && nRecvThreadStopProcessInfo == 1
			&& nMainLogicThreadStopProcessInfo == 1 && nEpollThreadStopProcessInfo == 1)
	{
		//	把该session放入可重用的Session内存池中
		m_pNetWorkServices->GetSessionManager().MoveWillCloseSessionToSessionPool(pSession);
	}
}


void LEpollThread::ReleaseEpollThreadResource()
{ 
	if (m_nEpollThreadHandle != -1)
	{
		close(m_nEpollThreadHandle);
	}
	if (m_pEvents != NULL)
	{
		delete[] m_pEvents;
		m_pEvents = NULL;
	}
}

