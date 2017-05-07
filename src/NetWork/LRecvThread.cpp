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

#include "LRecvThread.h"
#include "LNetWorkServices.h"
#include "LSessionManager.h"
#include "LSession.h"
#include "LSendThread.h"
#include <sys/prctl.h>



LRecvThread::LRecvThread()
{
	m_pNetWorkServices 		= NULL;
	m_nPacketRecved 			= 0;
	m_nThreadID 				= -1;
	m_tLastWriteBufDescTime = 0;


	m_pArrayRecvWorkOfEpollthreadToRecvthread = NULL;
	m_unEpollThreadCount								= 0;

	memset(m_szTempPacketData, 0, MAX_TEMP_PACKET_BUF_LEN);
	m_unPacketDataLen									= 0;

	m_nFPS						= 0;
	m_nCurrentSecondFPS		= 0;
	m_tLastSecond				= 0;
	m_nMinFPS					= 0x0FFFFFFF;
	m_nMaxFPS					= 0;
}

LRecvThread::~LRecvThread()
{
}


//	unEpollThreadCount 						epollThread数量
//	unWorItemCountperFixLenCircleLen 	每个epollThread可以提交的任务最大工作队列长度
//	unWorkItemCountOfCloseSessionFixLenCircleBuf	//	连接关闭工作队列长度
bool LRecvThread::Initialize(unsigned int unEpollThreadCount, unsigned int unWorItemCountperFixLenCircleBuf, unsigned int unWorkItemCountOfCloseSessionFixLenCircleBuf)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	if (unEpollThreadCount == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::Initialize", __LINE__,
															"unEpollThreadCount == 0\n");
		return false;
	}

	//	EpollThread环形缓冲池初始化
	if (!InitializeRecvwordOfFixLenCircleBuf(unEpollThreadCount, unWorItemCountperFixLenCircleBuf))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::Initialize", __LINE__,
																			"InitializeRecvwordOfFixLenCircleBuf Failed, EpollThreadCount:%u, WorkItemPerCircleBuf:%u\n", unEpollThreadCount, unWorItemCountperFixLenCircleBuf);
		return false;
	}
	//	关闭连接环形缓冲池初始化
	if (!m_FixBufWillCloseSessionToProcess.Initialize(sizeof(LSession*), unWorkItemCountOfCloseSessionFixLenCircleBuf))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::Initialize", __LINE__,
																			"m_FixBufWillCloseSessionToProcess.Initialize Failed, WorkItemCountOfCloseSessionFixLenCircleBuf:%u\n", unWorkItemCountOfCloseSessionFixLenCircleBuf);
		return false;
	}
	if (!m_FixBufFirstRecvOfSession.Initialize(sizeof(uint64_t), unWorkItemCountOfCloseSessionFixLenCircleBuf))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::Initialize", __LINE__,
				"m_FixBufFirstRecvOfSession.Initialize Failed, WorkItemCountOfCloseSessionFixLenCircleBuf:%u\n", unWorkItemCountOfCloseSessionFixLenCircleBuf);
		return false;
	}
	return true;
}
void LRecvThread::PrintRecvThreadStatus()
{
	m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LRecvThread::PrintRecvThreadStatus", __LINE__,
																													"\tRecvThread All BufferInfo, ThreadID:%d\n", m_nThreadID);
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		int nCurrentEpollInCount = m_pArrayRecvWorkOfEpollthreadToRecvthread[unIndex].GetCurrentExistCount();
		int nMaxEpollInCount = m_pArrayRecvWorkOfEpollthreadToRecvthread[unIndex].GetMaxItemCount();
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LRecvThread::PrintRecvThreadStatus", __LINE__,
								"\t\tRecvThread, ThreadID:%d, EpollThreadID:%u, EpollInWorkCount:%d, EpollInMaxCount:%d, PercentInfo:%f\n",
								m_nThreadID, unIndex + 1, nCurrentEpollInCount, nMaxEpollInCount, float(nCurrentEpollInCount) / float(nMaxEpollInCount));
	}

	int nCloseWorkItemCount = m_FixBufWillCloseSessionToProcess.GetCurrentExistCount();
	int nCloseMaxItemCount = m_FixBufWillCloseSessionToProcess.GetMaxItemCount();
	m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LRecvThread::PrintRecvThreadStatus", __LINE__,
									"\t\tRecvThread, ThreadID:%d, CloseWorkCount:%d, CloseMaxCount:%d, PercentInfo:%f\n",
									m_nThreadID, nCloseWorkItemCount, nCloseMaxItemCount, float(nCloseWorkItemCount) / float(nCloseMaxItemCount));

	int nFirstRecvWorkItemCount = m_FixBufFirstRecvOfSession.GetCurrentExistCount();
	int nFirstRecvMaxCount = m_FixBufFirstRecvOfSession.GetMaxItemCount();
	m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, m_nThreadID, __FILE__, "LRecvThread::PrintRecvThreadStatus", __LINE__,
										"\t\tRecvThread, ThreadID:%d, FirstRecvWorkCount:%d, FirstRecvMaxCount:%d, PercentInfo:%f\n",
										m_nThreadID, nFirstRecvWorkItemCount, nFirstRecvMaxCount, float(nFirstRecvWorkItemCount) / float(nFirstRecvMaxCount));
}

bool LRecvThread::InitializeRecvwordOfFixLenCircleBuf(unsigned int unEpollThreadCount, unsigned int unMaxWorkItemCount)
{
	if (unEpollThreadCount == 0 || unMaxWorkItemCount == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::InitializeRecvwordOfFixLenCircleBuf", __LINE__,
																					"EpollThreadCount:%u, MaxWorkItemCount:%u\n", unEpollThreadCount, unMaxWorkItemCount);
		return false;
	}
	m_unEpollThreadCount = unEpollThreadCount;
	m_pArrayRecvWorkOfEpollthreadToRecvthread = new LFixLenCircleBuf[m_unEpollThreadCount];
	if (m_pArrayRecvWorkOfEpollthreadToRecvthread == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::InitializeRecvwordOfFixLenCircleBuf", __LINE__,
																							"m_pArrayRecvWorkOfEpollthreadToRecvthread == NULL\n");
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		bool bSuccess = m_pArrayRecvWorkOfEpollthreadToRecvthread[unIndex].Initialize(sizeof(uint64_t), unMaxWorkItemCount);
		if (!bSuccess)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::InitializeRecvwordOfFixLenCircleBuf", __LINE__,
																										"m_pArrayRecvWorkOfEpollthreadToRecvthread[%u].Initialize, MaxWorkItemCount:%u\n", unIndex, unMaxWorkItemCount);
			return false;
		}
	}
	return true;
}

bool LRecvThread::AddRecvworkToRecvthread(unsigned int unEpollThreadID, uint64_t u64SessionID)
{
	if (u64SessionID == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::AddRecvworkToRecvthread", __LINE__,
																												"u64SessionID == 0\n");
		return false;
	}
	if (unEpollThreadID == 0 || unEpollThreadID > m_unEpollThreadCount)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::AddRecvworkToRecvthread", __LINE__,
																														"EpollThreadID:%u, EpollThreadCount:%u\n", unEpollThreadID, m_unEpollThreadCount);
		return false;
	}
	E_Circle_Error eCircleError = m_pArrayRecvWorkOfEpollthreadToRecvthread[unEpollThreadID - 1].AddItems((char*)&u64SessionID, 1);
	if (eCircleError != E_Circle_Buf_No_Error)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::AddRecvworkToRecvthread", __LINE__,
				"m_pArrayRecvWorkOfEpollthreadToRecvthread[%u].AddItems Failed, SessionID:%llu\n"
				, unEpollThreadID, u64SessionID);
		return false;
	}
	return true;
}

//	添加一个Session去关闭线程关闭
void LRecvThread::AddSessionToCloseThreadToClose(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}
	t_Client_Need_To_Close cntc;
	cntc.u64SessionID = pSession->GetSessionID();
	LCloseSocketThread* pCloseThread = &m_pNetWorkServices->GetCloseSocketThread();

	if (pSession->GetCloseWorkSendedToCloseThread() == 0)	//	如果没有提交过就提交，不需要重复提交
	{
		if (pCloseThread->AppendToClose(E_Thread_Type_Recv, m_nThreadID, cntc))
		{
			pSession->SetCloseWorkSendedToCloseThread(1);
		}
		else
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LSession::RecvData", __LINE__,
																											"AppendToClose Failed, SessionID:%llu\n", cntc.u64SessionID);
		}
	}
}

//	返回处理的数量
int LRecvThread::ProcessRecvworkFromEpollThread(unsigned int unDoWorkCountPerFixLenCircleBuf)
{
	int nProcessedCount = 0;
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		int nProcessedCountThisRound = 0;

		while(1)
		{
			uint64_t u64SessionID = 0;
			E_Circle_Error eCircleError = m_pArrayRecvWorkOfEpollthreadToRecvthread[unIndex].GetOneItem((char*)&u64SessionID, sizeof(u64SessionID));
			if (eCircleError == E_Circle_Buf_No_Error)
			{
				//	处理一个连接的接收工作
				ProcessRecvSystemCall(u64SessionID, false);
			}
			else if (eCircleError != E_Circle_Buf_Is_Empty)
			{
				m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::ProcessRecvworkFromEpollThread", __LINE__,
								"m_pArrayRecvWorkOfEpollthreadToRecvthread[%u].GetOneItem Failed, ErrorCode:%d\n"
								, unIndex, eCircleError);
			}
			else
			{
				break;
			}
			nProcessedCountThisRound++;
			if (nProcessedCountThisRound >= unDoWorkCountPerFixLenCircleBuf)
			{
				break;
			}
		}
		nProcessedCount += nProcessedCountThisRound;
	}
	return nProcessedCount;
}

void LRecvThread::ProcessFirstRecvOfSession()
{
	uint64_t u64SessionID = 0;
	int nProcessed = 0;
	while (1)
	{
		E_Circle_Error eCircleError = m_FixBufFirstRecvOfSession.GetOneItem((char*)&u64SessionID, sizeof(u64SessionID));
		if (eCircleError == E_Circle_Buf_No_Error)
		{
			//	处理一个连接的接收工作
			ProcessRecvSystemCall(u64SessionID, true);
			nProcessed++;
		}
		else
		{
			break;
		}
		if (nProcessed >= 200)
		{
			break;
		}
	}
}

int LRecvThread::ProcessRecvSystemCall(uint64_t u64SessionID, bool bIsFirstRecv)
{
	LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager();
	LSession* pSession = pMasterSessionManager->FindSession(u64SessionID);
	if (pSession != NULL)
	{
		//	没有人要关闭连接，才处理，否则就不处理了，等待关闭
		if (pSession->GetCloseWorkSendedToCloseThread() == 0)
		{
			pSession->RecvData(this, bIsFirstRecv);
		}
	}
	return 0;
}

int LRecvThread::SetTempPacketData(LSession* pSession, char* pData, unsigned int unDataLen)
{
	m_unPacketDataLen = 0;
	if (unDataLen >= MAX_TEMP_PACKET_BUF_LEN - sizeof(uint64_t))
	{
		return -2;
	}
	if (pSession == NULL || pData == NULL)
	{
		return -1;
	}
	if (unDataLen == 0)		//	数据上来只有头部，认定为保持通讯的KEEPALIVE数据包,根据具体的应用处理，如果认为只有头部的为非法数据，那么这里返回负数
	{
		return 1;
	}
	uint64_t u64SessionID = pSession->GetSessionID();
	memmove(m_szTempPacketData + m_unPacketDataLen, &u64SessionID, sizeof(uint64_t));
	m_unPacketDataLen += sizeof(uint64_t);

	memmove(m_szTempPacketData + m_unPacketDataLen, pData, unDataLen);
	m_unPacketDataLen += unDataLen;
	return 0;
}

char* LRecvThread::GetTempPacketDataBuf()
{
	return m_szTempPacketData;
}
unsigned int LRecvThread::GetTempPacketDataLen()
{
	return m_unPacketDataLen;
}

int LRecvThread::ThreadDoing(void* pParam)
{
	char szThreadName[128];
	sprintf(szThreadName, "RecvThread_%d", m_nThreadID);
	prctl(PR_SET_NAME, szThreadName);

	m_tLastWriteBufDescTime = time(NULL);

	while (true)
	{
		//	处理连接断开
		int nSessionWillToCloseProcessedCount = 0;
		LSession* pSession = NULL;
		while (GetOneWillCloseSession(&pSession))
		{
			nSessionWillToCloseProcessedCount++;

			if (pSession != NULL)
			{
				ProcessRecvDataErrorToCloseSession(pSession);
			}
			if (nSessionWillToCloseProcessedCount > 100)
			{
				break;
			}
		}

		//	处理连接的第一次接收
		ProcessFirstRecvOfSession();
		//	处理接收工作
		int nProcessedRecvWorkItemCount = ProcessRecvworkFromEpollThread(100);

		//	没有工作，休息
		if (nProcessedRecvWorkItemCount == 0 && nSessionWillToCloseProcessedCount == 0)
		{
			//	sched_yield();
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
		time_t timeNow = time(NULL);
		if (timeNow - m_tLastWriteBufDescTime > 60)  // 30秒记录一次缓存状态，测试使用
		{
			PrintRecvThreadStatus();
			m_tLastWriteBufDescTime = timeNow;
		}
		if (m_tLastSecond != timeNow)
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
			m_tLastSecond = timeNow;
		}
		else
		{
			m_nCurrentSecondFPS++;
		}
#endif
	}
	return 0;
}

bool LRecvThread::OnStart()
{
	return true;
}
void LRecvThread::OnStop()
{
	
}

void LRecvThread::SetNetServices(LNetWorkServices* pNetWorkServices)
{
	m_pNetWorkServices = pNetWorkServices;
}

void LRecvThread::ReleaseRecvThreadResource()
{
	if (m_pArrayRecvWorkOfEpollthreadToRecvthread != NULL)
	{
		delete[] m_pArrayRecvWorkOfEpollthreadToRecvthread;
		m_pArrayRecvWorkOfEpollthreadToRecvthread = NULL;
	}
}

//	加入一个需要处理的即将关闭的连接
void LRecvThread::AddWillCloseSession(LSession* pSession)
{
	E_Circle_Error eError = m_FixBufWillCloseSessionToProcess.AddItems((char*)&pSession, 1);
	if (eError != E_Circle_Buf_No_Error)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::AddWillCloseSession", __LINE__,
				"m_FixBufWillCloseSessionToProcess.AddItems 1 Failed, SessionID:%llu, ErrorID:%d\n", pSession->GetSessionID(), eError);
	}
}
//	取一个出来处理
bool LRecvThread::GetOneWillCloseSession(LSession** pSession)
{
	E_Circle_Error eError = m_FixBufWillCloseSessionToProcess.GetOneItem((char*)(&(*pSession)), sizeof(LSession*));
	if (eError != E_Circle_Buf_No_Error && eError != E_Circle_Buf_Is_Empty)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::GetOneWillCloseSession", __LINE__,
						"m_FixBufWillCloseSessionToProcess.GetOneItem 1 Failed, errorID:%d\n", eError);
		return false;
	}
	if (eError == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}

//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
void LRecvThread::ProcessRecvDataErrorToCloseSession(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}

	//	设置本接收线程不再对该连接进行操作
	pSession->SetRecvThreadStopProcess(1);

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

bool LRecvThread::AddFirstRecvOfSession(uint64_t u64SessionID)
{
	if (u64SessionID == 0)
	{
		return false;
	}
	E_Circle_Error eCircleError = m_FixBufFirstRecvOfSession.AddItems((char*)&u64SessionID, 1);
	if (eCircleError != E_Circle_Buf_No_Error)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_RECV_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, m_nThreadID, __FILE__, "LRecvThread::AddFirstRecvOfSession", __LINE__,
								"m_FixBufFirstRecvOfSession.AddItems 1 Failed, errorID:%d, SessionID:%llu\n", eCircleError, u64SessionID);
		return false;
	}
	return true;
}
