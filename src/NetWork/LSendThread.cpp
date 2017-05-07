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

#include "LSendThread.h"
#include "LSessionManager.h"
#include "LSession.h"
#include "LNetWorkServices.h"
#include "LCloseSocketThread.h" 
#include <sys/prctl.h>


LSendThread::LSendThread(void)
{
	m_pNetWorkServices 					= NULL;
	m_nThreadID 							= -1;
	m_nRealSendCount 						= 0;
	m_tLastWriteErrorTime 				= 0;
	m_nFreePacketCount 					= 0;
	m_pArrayEpollOutFromEpollThread 	= NULL;
	m_unEpollThreadCount					= 0;

	m_nFPS						= 0;
	m_nCurrentSecondFPS		= 0;
	m_tLastSecond				= 0;
	m_nMinFPS					= 0x0FFFFFFF;
	m_nMaxFPS					= 0;
	m_nMaxSendCountInSendCircleBuf	= 0;
}

LSendThread::~LSendThread(void)
{ 
}

//	unSendWorkItemCount 发送环形缓冲池大小，主线程可以放入的发送工作数量
//	unEpollThreadCount	EpollThread数量
//	unEpollOutEventMaxCount  EpollOut环形缓冲池队列的大小，每个epollThread对应一个环形缓冲池
//	unCloseWorkItemCount     CloseThread放入关闭工作队列大小
bool LSendThread::Initialize(unsigned int unSendWorkItemCount, unsigned int unEpollThreadCount, unsigned int unEpollOutEventMaxCount, unsigned int unCloseWorkItemCount)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}

	if (unSendWorkItemCount == 0 || unEpollThreadCount == 0 || unEpollOutEventMaxCount == 0 || unCloseWorkItemCount == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::Initialize", __LINE__,
																	"SendWorkItemCount:%u, EpollThreadCount:%u, EpollOutEventMaxCount:%u, CloseWorkItemCount:%u\n",
																	unSendWorkItemCount, unEpollThreadCount, unEpollOutEventMaxCount, unCloseWorkItemCount);
		return false;
	}
	if (!m_SendCircleBuf.Initialize(sizeof(uint64_t), unSendWorkItemCount))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::Initialize", __LINE__,
																			"m_SendCircleBuf.Initialize Failed, SendWorkItemCount:%u\n", unSendWorkItemCount);
		return false;
	}

	m_unEpollThreadCount = unEpollThreadCount;
	m_pArrayEpollOutFromEpollThread = new LFixLenCircleBuf[m_unEpollThreadCount];
	if (m_pArrayEpollOutFromEpollThread == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::Initialize", __LINE__,
																					"m_pArrayEpollOutFromEpollThread == NULL, EpollThreadCount:%u\n", m_unEpollThreadCount);
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		if (!m_pArrayEpollOutFromEpollThread[unIndex].Initialize(sizeof(uint64_t), unEpollOutEventMaxCount))
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::Initialize", __LINE__,
					"m_pArrayEpollOutFromEpollThread[%u].Initialize Failed, EpollOutEventMaxCount:%u\n", unIndex, unEpollOutEventMaxCount);
			return false;
		}
	}
	if (!m_FixCircleBufWillSessionCloseToProcessSendThread.Initialize(sizeof(LSession*), unCloseWorkItemCount))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::Initialize", __LINE__,
							"m_FixCircleBufWillSessionCloseToProcessSendThread.Initialize Failed, CloseWorkItemCount:%u\n", unCloseWorkItemCount);
		return false;
	}
	return true;
}
void LSendThread::PrintAllBufferInfo()
{
#if PRINT_INFO_TO_DEBUG
	m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LSendThread::PrintAllBufferInfo", __LINE__,
								"\tThread Buffer UseInfo, ThreadID:%d\n", m_nThreadID);
	int nExistSendCount = m_SendCircleBuf.GetCurrentExistCount();
	int nMaxSendCount = m_SendCircleBuf.GetMaxItemCount();
	m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LSendThread::PrintAllBufferInfo", __LINE__,
									"\t\tThreadID:%d, SendWorkCount:%d, SendWorkBufMaxCount:%d, PercentInfo:%f, MaxSendWorkItemCount:%d\n", m_nThreadID,
									nExistSendCount, nMaxSendCount, float(nExistSendCount) / float(nMaxSendCount), m_nMaxSendCountInSendCircleBuf);
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		int nExistCount = m_pArrayEpollOutFromEpollThread[unIndex].GetCurrentExistCount();
		int nMaxCount = m_pArrayEpollOutFromEpollThread[unIndex].GetMaxItemCount();
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LSendThread::PrintAllBufferInfo", __LINE__,
											"\t\tThreadID:%d, EpollThreadID:%u, EpollOutWorkCount:%d, EpollOutWorkBufMaxCount:%d, PercentInfo:%f\n",
											m_nThreadID, unIndex + 1, nExistCount, nMaxCount, float(nExistCount) / float(nMaxCount));
	}

	int nExistCloseCount = m_FixCircleBufWillSessionCloseToProcessSendThread.GetCurrentExistCount();
	int nMaxCloseCount = m_FixCircleBufWillSessionCloseToProcessSendThread.GetMaxItemCount();

	m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LSendThread::PrintAllBufferInfo", __LINE__,
												"\t\tThreadID:%d, CloseWorkCount:%d, CloseWorkBufMaxCount:%d, PerCentInfo:%f\n",
												m_nThreadID, nExistCloseCount, nMaxCloseCount, float(nExistCloseCount) / float(nMaxCloseCount));
#endif
}

bool LSendThread::AddOneSendWorkItem(uint64_t u64SessionID)
{
	E_Circle_Error eErrorCode = m_SendCircleBuf.AddItems((char*)&u64SessionID, 1);
	if (eErrorCode != E_Circle_Buf_No_Error)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::AddOneSendWorkItem", __LINE__,
									"m_SendCircleBuf.AddItems Failed, SessionID:%llu, ErrorCode:%d, CurrentItemCount:%d, MaxitemCount:%d\n", u64SessionID, eErrorCode, m_SendCircleBuf.GetCurrentExistCount(), m_SendCircleBuf.GetMaxItemCount());
		return false;
	}
	return true;
}

bool LSendThread::GetOneSendWorkItem(uint64_t& u64SessionID)
{
	E_Circle_Error eErrorCode = m_SendCircleBuf.GetOneItem((char*)&u64SessionID, sizeof(uint64_t));
	if (eErrorCode == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}

//	处理发送工作
int LSendThread::ProcessSendWork(int nMaxProcessSendWorkItem)
{
#if PRINT_INFO_TO_DEBUG
	int nCurCount = m_SendCircleBuf.GetCurrentExistCount();
	if (nCurCount > m_nMaxSendCountInSendCircleBuf)
	{
		m_nMaxSendCountInSendCircleBuf = nCurCount;
	}
#endif
	LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager();
	int nProcessedCount = 0;
	while(1)
	{
		uint64_t u64SessionID = 0;
		if (GetOneSendWorkItem(u64SessionID))
		{
			LSession* pSession = pMasterSessionManager->FindSession(u64SessionID);

			//	先将要发送的数据放入到连接的发送队列，然后将队列的数据拷贝到m_szSendThreadBuf中，然后用send发送
			//	这个改动的目的是，大量广播数据时 ，可以将数据拷贝到一起，节省send系统调用，原来逻辑是一个数据包使用
			//	一个send系统调用，测试后发现效率太低，大量广播数据存在时，连接经常因为数据无法及时发送出去而被关闭连接
			if (pSession != NULL)
			{
				if (pSession->GetCloseWorkSendedToCloseThread() == 0)
				{
					int nSendResult = pSession->SendDataInFixCircleSendBuf(this, false);
					if (nSendResult < 0)
					{
						m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::ProcessSendWork", __LINE__,
															"pSession->SendDataInFixCircleSendBuf Failed, SessionID:%llu, ErrorCode:%d\n", u64SessionID, nSendResult);
						//	这里需要关闭套接字
						t_Client_Need_To_Close tCntc;
						tCntc.u64SessionID = u64SessionID;
						if (m_pNetWorkServices->GetCloseSocketThread().AppendToClose(E_Thread_Type_Send, m_nThreadID, tCntc))
						{
							pSession->SetCloseWorkSendedToCloseThread(1);
						}
					}
					else
					{
						pSession->UpdateLastSendTime();
					}
				}
			}
			nProcessedCount++;
		}
		else
		{
			break;
		}

		if (nProcessedCount >= nMaxProcessSendWorkItem)
		{
			break;
		}
	}
	return nProcessedCount;
}

//	===============================================EPOLLOUT===================================BEGIN
bool LSendThread::AddEpollOutEvent(unsigned int unEpollThreadID, uint64_t u64SessionID)
{
	if (unEpollThreadID == 0 || unEpollThreadID > m_unEpollThreadCount)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::AddEpollOutEvent", __LINE__,
																	"EpollThreadID:%u, EpollThreadCount:%u\n", unEpollThreadID, m_unEpollThreadCount);
		return false;
	}
	E_Circle_Error eErrorCode = m_pArrayEpollOutFromEpollThread[unEpollThreadID - 1].AddItems((char*)&u64SessionID, 1);
	if (eErrorCode == E_Circle_Buf_No_Error)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::AddEpollOutEvent", __LINE__,
																			"m_pArrayEpollOutFromEpollThread[unEpollThreadID - 1].AddItems Failed, EpollThreadID:%u, EpollThreadCount:%u, ErrorCode:%d\n", unEpollThreadID, m_unEpollThreadCount, eErrorCode);
		return true;
	}
	return false;
}

//	处理EpollOut工作
int LSendThread::ProcessEpollOutEvent(unsigned int unMaxProcessCountPerFixLenCircleBuf)
{
	int nTotalProcessed = 0;
	LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager();

	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		int nCurProcessedCount = 0;
		while(1)
		{
			uint64_t u64SessionID = 0;
			if (GetOneEpollOutEvent(&m_pArrayEpollOutFromEpollThread[unIndex], u64SessionID))
			{
				LSession* pSession = pMasterSessionManager->FindSession(u64SessionID);

				if (pSession != NULL)
				{
					if (pSession->GetCloseWorkSendedToCloseThread() == 0)
					{
						int nSendResult = pSession->SendDataInFixCircleSendBuf(this, true);
						if (nSendResult < 0)
						{
							m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::ProcessEpollOutEvent", __LINE__,
																								"pSession->SendDataInFixCircleSendBuf Failed, Errcode:%d, SessionID:%llu\n", nSendResult, u64SessionID);
							t_Client_Need_To_Close tCntc;
							tCntc.u64SessionID = u64SessionID;
							//	这里需要关闭套接字
							if (m_pNetWorkServices->GetCloseSocketThread().AppendToClose(E_Thread_Type_Send, m_nThreadID, tCntc))
							{
								pSession->SetCloseWorkSendedToCloseThread(1);
							}
						}
						else
						{
							pSession->UpdateLastSendTime();
						}
					}
				}
				nCurProcessedCount++;
			}
			else
			{
				break;
			}
			if (nCurProcessedCount > unMaxProcessCountPerFixLenCircleBuf)
			{
				break;
			}
		}
		nTotalProcessed += nCurProcessedCount;
	}
	return nTotalProcessed;
}

bool LSendThread::GetOneEpollOutEvent(LFixLenCircleBuf* pFixLenCircleBuf, uint64_t& u64SessionID)
{
	if (pFixLenCircleBuf == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::GetOneEpollOutEvent", __LINE__,
											"pFixLenCircleBuf == NULL\n");
		return false;
	}
	E_Circle_Error eErrorCode = pFixLenCircleBuf->GetOneItem((char*)&u64SessionID, sizeof(uint64_t));
	if (eErrorCode == E_Circle_Buf_No_Error)
	{
		return true;
	}
	if (eErrorCode != E_Circle_Buf_Is_Empty)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::GetOneEpollOutEvent", __LINE__,
													"pFixLenCircleBuf->GetOneItem Failed, ErrorID:%d\n", eErrorCode);
	}
	return false;
}
//	==============================================================Epollout================END

int LSendThread::ThreadDoing(void* pParam)
{
	char szThreadName[128];
	sprintf(szThreadName, "SendThread_%d", m_nThreadID);
	prctl(PR_SET_NAME, szThreadName);

	m_tLastWriteErrorTime = time(NULL);

	while(true)
	{
		int nWillCloseSessionProcessed = 0;
		LSession* pWillCloseSession = NULL;
		while (GetOneWillCloseSessionInSendThread(&pWillCloseSession))
		{
			nWillCloseSessionProcessed++;
			if (pWillCloseSession != NULL)
			{
				ProcessSendDataErrorToCloseSession(pWillCloseSession);
			}
			if (nWillCloseSessionProcessed >= 100)
			{
				break;
			}
		}

		int nEpollOutEventProcessed = ProcessEpollOutEvent(100);
		int nSendProcessed = ProcessSendWork(100);

		if (nWillCloseSessionProcessed == 0
				&& nEpollOutEventProcessed == 0
				&& nSendProcessed == 0)
		{
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		if (CheckForStop())
		{
			break;
		}
#if PRINT_INFO_TO_DEBUG
		time_t tNow = time(NULL);
		if (tNow - m_tLastWriteErrorTime > 50)
		{
			PrintAllBufferInfo();
			m_tLastWriteErrorTime = tNow;
		}
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
	}
	return 0;
}
bool LSendThread::OnStart()
{
	return true;
}
void LSendThread::OnStop()
{
}

void LSendThread::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
{
	m_pNetWorkServices =  pNetWorkServices;
}
LNetWorkServices* LSendThread::GetNetWorkServices()	
{
	return m_pNetWorkServices;
}

void LSendThread::PrintSendThreadLocalBufStatus()
{

}

void LSendThread::ReleaseSendThreadResource()
{
	if (m_pArrayEpollOutFromEpollThread != NULL)
	{
		delete[] m_pArrayEpollOutFromEpollThread;
		m_pArrayEpollOutFromEpollThread = NULL;
	}
}

//	加入一个需要处理的即将关闭的连接
void LSendThread::AddWillCloseSessionInSendThread(LSession* pSession)
{
	E_Circle_Error eError = m_FixCircleBufWillSessionCloseToProcessSendThread.AddItems((char*)&pSession, 1);
	if (eError != E_Circle_Buf_No_Error)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::AddWillCloseSessionInSendThread", __LINE__,
															"m_FixCircleBufWillSessionCloseToProcessSendThread.AddItems Failed, ErrorID:%d\n", eError);
	}
}
//	取一个出来处理
bool LSendThread::GetOneWillCloseSessionInSendThread(LSession** pSession)
{
	E_Circle_Error eError = m_FixCircleBufWillSessionCloseToProcessSendThread.GetOneItem((char*)(&(*pSession)), sizeof(LSession*));
	if (eError != E_Circle_Buf_No_Error && eError != E_Circle_Buf_Is_Empty)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_SEND_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSendThread::GetOneWillCloseSessionInSendThread", __LINE__,
																	"m_FixCircleBufWillSessionCloseToProcessSendThread.GetOneItem Failed, ErrorID:%d\n", eError);
		return false;
	}
	if (eError == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}

//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
void LSendThread::ProcessSendDataErrorToCloseSession(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}

	//	设置本接收线程不再对该连接进行操作
	pSession->SetSendThreadStopProcess(1);

	//	检查是否需要关闭连接
	int nSendThreadStopProcessInfo 		= 0;
	int nRecvThreadStopProcessInfo		= 0;
	int nMainLogicThreadStopProcessInfo	= 0;
	int nEpollThreadStopProcessInfo		= 0;
	pSession->GetStopProcessInfos(nSendThreadStopProcessInfo, nRecvThreadStopProcessInfo
			, nMainLogicThreadStopProcessInfo, nEpollThreadStopProcessInfo);
	if (nSendThreadStopProcessInfo == 1 && nRecvThreadStopProcessInfo == 1
			&& nMainLogicThreadStopProcessInfo == 1 && nEpollThreadStopProcessInfo == 1)
	{
		m_pNetWorkServices->GetSessionManager().MoveWillCloseSessionToSessionPool(pSession);
	}
}

