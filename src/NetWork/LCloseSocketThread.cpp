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

#include "LCloseSocketThread.h"
#include "LNetWorkServices.h"
#include "LRecvThreadManager.h"
#include "LSendThreadManager.h"
#include "LEpollThreadManager.h"
#include "LSessionManager.h"
#include "LSession.h"
#include "IncludeHeader.h"
#include "LEpollThread.h"
#include "LSendThread.h"
#include "LRecvThread.h"
#include <sys/prctl.h>

LCloseSocketThread::LCloseSocketThread()
{
	m_unAcceptThreadCount		= 0;
	m_unEpollThreadCount			= 0;
	m_unRecvThreadCount			= 0;
	m_unSendThreadCount			= 0;
	m_unMainLogicThreadCount	= 0;

	m_pArrayBufSessionNeedToCloseOfAcceptThread 		= NULL;
	m_pArrayBufSessionNeedToCloseOfEpollThread 		= NULL;
	m_pArrayBufSessionNeedToCloseOfRecvThread 		= NULL;
	m_pArrayBufSessionNeedToCloseOfSendThread 		= NULL;
	m_pArrayBufSessionNeedToCloseOfMainLogicThread	= NULL;

	m_pNetWorkServices 	= NULL;
	m_nThreadID				= 0;
	m_tLastPrintBufferTime = 0;
}

LCloseSocketThread::~LCloseSocketThread()
{
}


//	unCloseWorkItemCount 最大可以提交的关闭事件数量
//	ppdForLocalPool 连接关闭时，释放该连接下没有发送的发送数据包到本地缓存池，达到一定数量时，提交到全局缓冲池
//	unppdForLocalPoolCount 描述信息数组的长度
bool LCloseSocketThread::Initialize(int nThreadID, unsigned int unAcceptThreadCount, unsigned int unEpollThreadCount, unsigned int unRecvThreadCount, unsigned int unSendThreadCount,
		unsigned int unMainLogicThreadCount, unsigned int unMaxWorkItemForClose)
{
	if (m_pNetWorkServices == NULL || nThreadID <= 0)
	{
		return false;
	}
	m_nThreadID = nThreadID;

	if (unAcceptThreadCount == 0 || unEpollThreadCount == 0 || unRecvThreadCount == 0 || unSendThreadCount == 0 || unMainLogicThreadCount == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
									"AcceptThreadCount:%u, EpollThreadCount:%u, RecvThreadCount:%u, SendThreadCount:%u, MainLogicThreadCount:%u\n",
				unAcceptThreadCount, unEpollThreadCount, unRecvThreadCount, unSendThreadCount, unMainLogicThreadCount);
		return false;
	}
	m_unAcceptThreadCount		= unAcceptThreadCount;
	m_unEpollThreadCount			= unEpollThreadCount;
	m_unRecvThreadCount			= unRecvThreadCount;
	m_unSendThreadCount			= unSendThreadCount;
	m_unMainLogicThreadCount	= unMainLogicThreadCount;

	m_pArrayBufSessionNeedToCloseOfAcceptThread 		= new LFixLenCircleBuf[m_unAcceptThreadCount];
	if (m_pArrayBufSessionNeedToCloseOfAcceptThread == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
									"m_pArrayBufSessionNeedToCloseOfAcceptThread == NULL, AcceptThreadCount:%u\n", m_unAcceptThreadCount);
		return false;
	}
	if (!InitializeFixLenCircleArray(m_pArrayBufSessionNeedToCloseOfAcceptThread, m_unAcceptThreadCount, sizeof(t_Client_Need_To_Close), unMaxWorkItemForClose))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
											"InitializeFixLenCircleArray For AcceptThread Failed, AcceptThreadCount:%u, MaxWorkItemForClose:%u\n", m_unAcceptThreadCount, unMaxWorkItemForClose);
		return false;
	}

	m_pArrayBufSessionNeedToCloseOfEpollThread 		= new LFixLenCircleBuf[m_unEpollThreadCount];
	if (m_pArrayBufSessionNeedToCloseOfEpollThread == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
											"m_pArrayBufSessionNeedToCloseOfEpollThread == NULL, EpollThreadCount:%u\n", m_unEpollThreadCount);
		return false;
	}
	if (!InitializeFixLenCircleArray(m_pArrayBufSessionNeedToCloseOfEpollThread, m_unEpollThreadCount, sizeof(t_Client_Need_To_Close), unMaxWorkItemForClose))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
													"InitializeFixLenCircleArray For EpollThread Failed, EpollThreadCount:%u, MaxWorkItemForClose:%u\n", m_unEpollThreadCount, unMaxWorkItemForClose);
		return false;
	}

	m_pArrayBufSessionNeedToCloseOfRecvThread 		= new LFixLenCircleBuf[m_unRecvThreadCount];
	if(m_pArrayBufSessionNeedToCloseOfRecvThread == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
													"m_pArrayBufSessionNeedToCloseOfRecvThread == NULL, RecvThreadCount:%u\n", m_unRecvThreadCount);
		return false;
	}
	if (!InitializeFixLenCircleArray(m_pArrayBufSessionNeedToCloseOfRecvThread, m_unRecvThreadCount, sizeof(t_Client_Need_To_Close), unMaxWorkItemForClose))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
													"InitializeFixLenCircleArray For RecvThread Failed, RecvThreadCount:%u, MaxWorkItemForClose:%u\n", m_unRecvThreadCount, unMaxWorkItemForClose);
		return false;
	}

	m_pArrayBufSessionNeedToCloseOfSendThread 		= new LFixLenCircleBuf[m_unSendThreadCount];
	if (m_pArrayBufSessionNeedToCloseOfSendThread == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
															"m_pArrayBufSessionNeedToCloseOfSendThread == NULL, SendThreadCount:%u\n", m_unSendThreadCount);
		return false;
	}
	if (!InitializeFixLenCircleArray(m_pArrayBufSessionNeedToCloseOfSendThread, m_unSendThreadCount, sizeof(t_Client_Need_To_Close), unMaxWorkItemForClose))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
													"InitializeFixLenCircleArray For SendThread Failed, SendThreadCount:%u, MaxWorkItemForClose:%u\n", m_unSendThreadCount, unMaxWorkItemForClose);
		return false;
	}
	m_pArrayBufSessionNeedToCloseOfMainLogicThread	= new LFixLenCircleBuf[m_unMainLogicThreadCount];
	if (m_pArrayBufSessionNeedToCloseOfMainLogicThread == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
															"m_pArrayBufSessionNeedToCloseOfMainLogicThread == NULL, MainLogicThreadCount:%u\n", m_unMainLogicThreadCount);

		return false;
	}
	if (!InitializeFixLenCircleArray(m_pArrayBufSessionNeedToCloseOfMainLogicThread, m_unMainLogicThreadCount, sizeof(t_Client_Need_To_Close), unMaxWorkItemForClose))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::Initialize", __LINE__,
													"InitializeFixLenCircleArray For MainLogicThread Failed, MainLogicThreadCount:%u, MaxWorkItemForClose:%u\n", m_unMainLogicThreadCount, unMaxWorkItemForClose);
		return false;
	}

	return true;
}
void LCloseSocketThread::PrintBufferInfos()
{
	m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LCloseSocketThread::PrintBufferInfos", __LINE__,
											"\tCloseThread BufferInfos\n");
	for (unsigned int unIndex = 0; unIndex < m_unAcceptThreadCount; ++unIndex)
	{
		int nCloseWorkCount = m_pArrayBufSessionNeedToCloseOfAcceptThread[unIndex].GetCurrentExistCount();
		int nCloseMaxCount = m_pArrayBufSessionNeedToCloseOfAcceptThread[unIndex].GetMaxItemCount();
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LCloseSocketThread::PrintBufferInfos", __LINE__,
													"\t\tAcceptCloseWorkCount:%d, AcceptCloseMaxCount:%d, PercentInfo:%f\n",
													nCloseWorkCount, nCloseMaxCount, float(nCloseWorkCount) / float(nCloseMaxCount));
	}

	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		int nCloseWorkCount = m_pArrayBufSessionNeedToCloseOfEpollThread[unIndex].GetCurrentExistCount();
		int nCloseMaxCount = m_pArrayBufSessionNeedToCloseOfEpollThread[unIndex].GetMaxItemCount();
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LCloseSocketThread::PrintBufferInfos", __LINE__,
													"\t\tEpollThreadID:%u, EpollCloseWorkCount:%d, EpollCloseMaxCount:%d, PercentInfo:%f\n",
													unIndex + 1, nCloseWorkCount, nCloseMaxCount, float(nCloseWorkCount) / float(nCloseMaxCount));
	}

	for (unsigned int unIndex = 0; unIndex < m_unRecvThreadCount; ++unIndex)
	{
		int nCloseWorkCount = m_pArrayBufSessionNeedToCloseOfRecvThread[unIndex].GetCurrentExistCount();
		int nCloseMaxCount = m_pArrayBufSessionNeedToCloseOfRecvThread[unIndex].GetMaxItemCount();
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LCloseSocketThread::PrintBufferInfos", __LINE__,
													"\t\tRecvThreadID:%u, RecvCloseWorkCount:%d, RecvCloseMaxCount:%d, PercentInfo:%f\n",
													unIndex + 1, nCloseWorkCount, nCloseMaxCount, float(nCloseWorkCount) / float(nCloseMaxCount));
	}

	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		int nCloseWorkCount = m_pArrayBufSessionNeedToCloseOfSendThread[unIndex].GetCurrentExistCount();
		int nCloseMaxCount = m_pArrayBufSessionNeedToCloseOfSendThread[unIndex].GetMaxItemCount();
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LCloseSocketThread::PrintBufferInfos", __LINE__,
													"\t\tSendThreadID:%u, SendCloseWorkCount:%d, SendCloseMaxCount:%d, PercentInfo:%f\n",
													unIndex + 1, nCloseWorkCount, nCloseMaxCount, float(nCloseWorkCount) / float(nCloseMaxCount));
	}

	for (unsigned int unIndex = 0; unIndex < m_unMainLogicThreadCount; ++unIndex)
	{
		int nCloseWorkCount = m_pArrayBufSessionNeedToCloseOfMainLogicThread[unIndex].GetCurrentExistCount();
		int nCloseMaxCount = m_pArrayBufSessionNeedToCloseOfMainLogicThread[unIndex].GetMaxItemCount();
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LCloseSocketThread::PrintBufferInfos", __LINE__,
													"\t\tMainLogicThreadID:%u, MainLogicCloseWorkCount:%d, MainLogicCloseMaxCount:%d, PercentInfo:%f\n",
													unIndex + 1, nCloseWorkCount, nCloseMaxCount, float(nCloseWorkCount) / float(nCloseMaxCount));
	}
}
bool LCloseSocketThread::InitializeFixLenCircleArray(LFixLenCircleBuf* pArrayFixLenCircleBuf, unsigned int unArraySize, unsigned int unItemSize, unsigned int unFixLenCircleItemCount)
{
	if (pArrayFixLenCircleBuf == NULL || unArraySize == 0 || unItemSize == 0 || unFixLenCircleItemCount == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::InitializeFixLenCircleArray", __LINE__,
				"ArraySize:%u, ItemSize:%u, FixLenCircleItemCount:%u\n", unArraySize, unItemSize, unFixLenCircleItemCount);
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < unArraySize; ++unIndex)
	{
		bool bSuccess = pArrayFixLenCircleBuf[unIndex].Initialize(unItemSize, unFixLenCircleItemCount);
		if (!bSuccess)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::InitializeFixLenCircleArray", __LINE__,
							"pArrayFixLenCircleBuf[%u].Initialize Failed, ArraySize:%u, ItemSize:%u, FixLenCircleItemCount:%u\n", unIndex, unArraySize, unItemSize, unFixLenCircleItemCount);
			return false;
		}
	}
	return true;
}
bool LCloseSocketThread::AppendToClose(E_Thread_Type eThreadType, unsigned int unThreadID, t_Client_Need_To_Close ClientToClose)
{
	if (eThreadType == E_Thread_Type_Accept)
	{
		if (unThreadID > m_unAcceptThreadCount || unThreadID == 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
										"unThreadID > m_unAcceptThreadCount || unThreadID == 0, ThreadID:%u, SessionID:%llu\n", unThreadID, ClientToClose.u64SessionID);
			return false;
		}
		E_Circle_Error nError = m_pArrayBufSessionNeedToCloseOfAcceptThread[unThreadID - 1].AddItems((char*)&ClientToClose, 1);
		if (nError != E_Circle_Buf_No_Error)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
													"m_pArrayBufSessionNeedToCloseOfAcceptThread.AddItems Failed, ThreadID:%u, SessionID:%llu, ErrorCode:%d\n",
													unThreadID, ClientToClose.u64SessionID, nError);
			return false;
		}
	}
	else if (eThreadType == E_Thread_Type_Epoll)
	{
		if (unThreadID > m_unEpollThreadCount || unThreadID == 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
													"unThreadID > m_unEpollThreadCount || unThreadID == 0, ThreadID:%u, SessionID:%llu\n", unThreadID, ClientToClose.u64SessionID);
			return false;
		}
		E_Circle_Error nError = m_pArrayBufSessionNeedToCloseOfEpollThread[unThreadID - 1].AddItems((char*)&ClientToClose, 1);
		if (nError != E_Circle_Buf_No_Error)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
													"m_pArrayBufSessionNeedToCloseOfEpollThread.AddItems Failed, ThreadID:%u, SessionID:%llu, ErrorCode:%d\n",
													unThreadID, ClientToClose.u64SessionID, nError);
			return false;
		}
	}
	else if (eThreadType == E_Thread_Type_Recv)
	{
		if (unThreadID > m_unRecvThreadCount || unThreadID == 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
																"unThreadID > m_unRecvThreadCount || unThreadID == 0, ThreadID:%u, SessionID:%llu\n", unThreadID, ClientToClose.u64SessionID);
			return false;
		}
		E_Circle_Error nError = m_pArrayBufSessionNeedToCloseOfRecvThread[unThreadID - 1].AddItems((char*)&ClientToClose, 1);
		if (nError != E_Circle_Buf_No_Error)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
																"m_pArrayBufSessionNeedToCloseOfRecvThread.AddItems Failed, ThreadID:%u, SessionID:%llu, ErrorCode:%d\n",
																unThreadID, ClientToClose.u64SessionID, nError);
			return false;
		}
	}
	else if (eThreadType == E_Thread_Type_Send)
	{
		if (unThreadID > m_unSendThreadCount || unThreadID == 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
																			"unThreadID > m_unSendThreadCount || unThreadID == 0, ThreadID:%u, SessionID:%llu\n", unThreadID, ClientToClose.u64SessionID);
			return false;
		}
		E_Circle_Error nError = m_pArrayBufSessionNeedToCloseOfSendThread[unThreadID - 1].AddItems((char*)&ClientToClose, 1);
		if (nError != E_Circle_Buf_No_Error)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
																			"m_pArrayBufSessionNeedToCloseOfSendThread.AddItems Failed, ThreadID:%u, SessionID:%llu, ErrorCode:%d\n",
																			unThreadID, ClientToClose.u64SessionID, nError);
			return false;
		}
	}
	else if (eThreadType == E_Thread_Type_MainLogic)
	{
		if (unThreadID > m_unMainLogicThreadCount || unThreadID == 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
																			"unThreadID > m_unMainLogicThreadCount || unThreadID == 0, ThreadID:%u, SessionID:%llu\n", unThreadID, ClientToClose.u64SessionID);
			return false;
		}
		E_Circle_Error nError = m_pArrayBufSessionNeedToCloseOfMainLogicThread[unThreadID - 1].AddItems((char*)&ClientToClose, 1);
		if (nError != E_Circle_Buf_No_Error)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::AppendToClose", __LINE__,
																						"m_pArrayBufSessionNeedToCloseOfMainLogicThread.AddItems Failed, ThreadID:%u, SessionID:%llu, ErrorCode:%d\n",
																						unThreadID, ClientToClose.u64SessionID, nError);
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

int LCloseSocketThread::ThreadDoing(void* pParam)
{
	if (m_pNetWorkServices == NULL)
	{
		return -1;
	}
	char szThreadName[128];
	sprintf(szThreadName, "CloseThread");
	prctl(PR_SET_NAME, szThreadName);

	m_tLastPrintBufferTime = time(NULL);

	while (1)
	{
		int nProcessedCount = 0;
		int nMaxProcessPerFixLenCircleBuf = 50;
		for (unsigned int unIndex = 0; unIndex < m_unAcceptThreadCount; ++unIndex)
		{
			nProcessedCount += ProcessDisconnectInFixLenCircle(&m_pArrayBufSessionNeedToCloseOfAcceptThread[unIndex], nMaxProcessPerFixLenCircleBuf);
		}

		for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
		{
			nProcessedCount += ProcessDisconnectInFixLenCircle(&m_pArrayBufSessionNeedToCloseOfEpollThread[unIndex], nMaxProcessPerFixLenCircleBuf);
		}

		for (unsigned int unIndex = 0; unIndex < m_unRecvThreadCount; ++unIndex)
		{
			nProcessedCount += ProcessDisconnectInFixLenCircle(&m_pArrayBufSessionNeedToCloseOfRecvThread[unIndex], nMaxProcessPerFixLenCircleBuf);
		}

		for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
		{
			nProcessedCount += ProcessDisconnectInFixLenCircle(&m_pArrayBufSessionNeedToCloseOfSendThread[unIndex], nMaxProcessPerFixLenCircleBuf);
		}

		for (unsigned int unIndex = 0; unIndex < m_unMainLogicThreadCount; ++unIndex)
		{
			nProcessedCount += ProcessDisconnectInFixLenCircle(&m_pArrayBufSessionNeedToCloseOfMainLogicThread[unIndex], nMaxProcessPerFixLenCircleBuf);
		}

		if (nProcessedCount == 0)
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
		time_t tNow = time(NULL);
		if (tNow - m_tLastPrintBufferTime > 120)
		{
			PrintBufferInfos();
			m_tLastPrintBufferTime = tNow;
		}
#endif
	}
	return 0;
}
//	处理在队列中的需要断开的连接，返回处理的数量
int LCloseSocketThread::ProcessDisconnectInFixLenCircle(LFixLenCircleBuf* pFixLenCircle, int nMaxProcessCount)
{
	if (pFixLenCircle == NULL)
	{
		return 0;
	}
	int nProcessedCount = 0;

	t_Client_Need_To_Close OneWorkItem;
	while (pFixLenCircle->GetOneItem((char*)&OneWorkItem, sizeof(OneWorkItem)) == E_Circle_Buf_No_Error)
	{
		nProcessedCount++;

		uint64_t u64SessionID = OneWorkItem.u64SessionID;
		LSession* pSession = m_pNetWorkServices->GetSessionManager().FindSession(u64SessionID);
		if (pSession != NULL)
		{
			ProcessDisconnect(pSession);
		}

		if (nProcessedCount >= nMaxProcessCount)
		{
			break;
		}
	}
	return nProcessedCount;
}

bool LCloseSocketThread::ProcessDisconnect(LSession* pSession)
{
	if (pSession == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::ProcessDisconnect", __LINE__,
																					"pSession == NULL\n");
		return false;
	}
	uint64_t u64SessionID = pSession->GetSessionID();

	//	改变Session对应slot的SessionIndex
	m_pNetWorkServices->GetSessionManager().FreeSession(pSession);

	//	将该线程添加到连接管理器里面的即将重用的管理器里面去
	m_pNetWorkServices->GetSessionManager().AddToWillReuseManager(pSession);

	//	通知所有操作Session的线程，停止处理该Session(原理是：session从session管理器里面移除了，那么其他线程无法再获取到Session，只要处理了该信息，那么对应的线程就不会再处理了)
	m_pNetWorkServices->AddWillCloseSessionInMainLogic(pSession);

	//	获取不到线程，有可能是acceptThread处理时，没有处理完关闭了
	int nEpollThreadID = pSession->GetEpollThreadID();
	if (nEpollThreadID > 0)
	{
		LEpollThread* pEpollThread = m_pNetWorkServices->GetEpollThreadManager().GetEpollThread(nEpollThreadID);
		if (pEpollThread != NULL)
		{
			pEpollThread->AddWillCloseSessionInEpollThread(pSession);
		}
		else
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::ProcessDisconnect", __LINE__,
																								"pEpollThread == NULL, EpollThreadIDOfSession:%d\n", nEpollThreadID);
		}
	}
	int nRecvThreadID = pSession->GetRecvThreadID();
	if (nRecvThreadID > 0)
	{
		LRecvThread* pRecvThread = m_pNetWorkServices->GetRecvThreadManager().GetRecvThread(nRecvThreadID);
		if (pRecvThread != NULL)
		{
			pRecvThread->AddWillCloseSession(pSession);
		}
		else
		{
			//error log;
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::ProcessDisconnect", __LINE__,
																											"pRecvThread == NULL, RecvThreadIDOfSession:%d\n", nRecvThreadID);
		}
	}
	int nSendThreadID = pSession->GetSendThreadID();
	if (nSendThreadID > 0)
	{
		LSendThread* pSendThread = m_pNetWorkServices->GetSendThreadManager().GetSendThread(nSendThreadID);
		if (pSendThread != NULL)
		{
			pSendThread->AddWillCloseSessionInSendThread(pSession);
		}
		else
		{
			//	error log
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::ProcessDisconnect", __LINE__,
																														"pSendThread == NULL, SendThreadIDOfSession:%d\n", nSendThreadID);
		}
	}

	bool bSuccessed = m_pNetWorkServices->AddRecvedPacketToMainLogicVarLenCircleBuf(E_Thread_Type_Close, m_nThreadID, (char*)&u64SessionID, sizeof(uint64_t));
	if (!bSuccessed)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::ProcessDisconnect", __LINE__,
																																"m_pNetWorkServices->AddRecvedPacketToMainLogicFixLenCircleBuf Failed, Session:%llu\n", u64SessionID);
		return false;
	}
	return true;
}
bool LCloseSocketThread::OnStart()
{
	return true;
}
void LCloseSocketThread::OnStop()
{
}

void LCloseSocketThread::ReleaseCloseSocketThreadResource()
{
}


void LCloseSocketThread::StopCloseSocketThread()
{
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		Stop();

		int nJoinRes = pthread_join(pID, NULL);
		if (nJoinRes != 0)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_CLOSE_THREAD, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LCloseSocketThread::StopCloseSocketThread", __LINE__,
																																			"pthread_join Failed, ErrorId:%d\n", errno);
		}
	}
}


