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

#include "LNetWorkServices.h"
#include "LSendThread.h"
#include "LAutoReleaseMutex.h"
#include "LServerBaseNetWork.h"

bool g_IsLittleEndian = true;

LNetWorkServices::LNetWorkServices()
{
	m_pArrayAcceptThreadRecvedPacket = NULL;
	m_pArrayRecvThreadRecvedPacket	= NULL;
	m_pArrayCloseThreadRecvedPacket	= NULL;
	m_unSendThreadCount					= 0;
	m_unAcceptThreadCount				= 0;
	m_unRecvThreadCount					= 0;
	m_unCloseThreadCount					= 0;
	m_unEpollThreadCount					= 0;
	m_pServerBaseNetWork					= NULL;
	m_pInfoRecorderBase					= NULL;
	m_tLastPrintInfosTime				= time(NULL);

	//	获取Accept数据包
	memset(m_szTempAcceptedSessionInfo, 0, MAX_DATA_BUF_LEN_TO_WRITE_ACCEPTED_SESSION_INFO);
	m_unTempAcceptedSessionInfoLen	= 0;

	//	获取客户端发送上来的数据包
	memset(m_szTempPacketData, 0, MAX_TEMP_PACKET_BUF_LEN);			//	临时存放当前的数据包
	m_unTempPacketDataLen				= 0;

	memset(m_szTempClosePacketData, 0, 32);								//	获取断开连接数据包
	m_unTempClosePacketDataLen			= 0;

	m_nProcessedThisRound				= 0;
}

void LNetWorkServices::SetServerBaseNetWork(LServerBaseNetWork* pServerBaseNetWork)
{
	m_pServerBaseNetWork	= pServerBaseNetWork;
}

LNetWorkServices::~LNetWorkServices()
{
}

LRecvThreadManager& LNetWorkServices::GetRecvThreadManager()
{
	return m_RecvThreadManager;
}
LSendThreadManager& LNetWorkServices::GetSendThreadManager()
{
	return m_SendThreadManager;
}
LEpollThreadManager& LNetWorkServices::GetEpollThreadManager()
{
	return m_EpollThreadManager;
}
LAcceptThread& LNetWorkServices::GetAcceptThread()
{
	return m_AcceptThread;
}

LMasterSessionManager& LNetWorkServices::GetSessionManager()
{
	return m_SessionManager;
}

LCloseSocketThread& LNetWorkServices::GetCloseSocketThread()
{
	return m_CloseSocketThread;
}

void LNetWorkServices::CheckMachineIsLittleEndian()
{
	union
	{
		unsigned int  a;
		unsigned char b;
	}c;
	c.a = 1;
	g_IsLittleEndian = (1 == c.b);
}
bool LNetWorkServices::Initialize(t_NetWorkServices_Params& nwsp, bool bInitialAccept)
{
	//	确定机器的字节序
	CheckMachineIsLittleEndian();

	m_RecvThreadManager.SetNetWorkServices(this);
	if (!m_RecvThreadManager.Init(nwsp.nwspEpollThread.unEpollThreadCount, nwsp.nwspRecvThread.usThreadCount, nwsp.nwspRecvThread.unRecvWorkItemCount, nwsp.nwspSession.unCloseWorkItemCount))
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Initialize", __LINE__,
				"m_RecvThreadManager.Init  Failed\n");
		return false;
	}
	m_unRecvThreadCount = nwsp.nwspRecvThread.usThreadCount;

	m_SendThreadManager.SetNetWorkServices(this);
	if (!m_SendThreadManager.Initialize(nwsp.nwspEpollThread.unEpollThreadCount, nwsp.nwspSendThread.unSendThreadCount, nwsp.nwspSendThread.unSendWorkItemCount, nwsp.nwspSendThread.unEpollOutEventMaxCount, nwsp.nwspSession.unCloseWorkItemCount))
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Initialize", __LINE__,
						"m_SendThreadManager.Initialize  Failed\n");
		return false;
	}
	m_unSendThreadCount = nwsp.nwspSendThread.unSendThreadCount;

	m_EpollThreadManager.SetNetWorkServices(this);
	if (!m_EpollThreadManager.Initialize(nwsp.nwspEpollThread.unEpollThreadCount, nwsp.nwspEpollThread.unWaitClientSizePerEpoll, nwsp.nwspSession.unCloseWorkItemCount))
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Initialize", __LINE__,
								"m_EpollThreadManager.Initialize  Failed\n");
		return false;
	}
	m_unEpollThreadCount = nwsp.nwspEpollThread.unEpollThreadCount;

	m_SessionManager.SetNetWorkServices(this);
	if (!m_SessionManager.InitializeSessionPool(nwsp.nwspSession.unMaxSessionNum, nwsp.nwspSession.unMaxPacketLen, nwsp.nwspSession.unSendBufLen))
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Initialize", __LINE__,
												"m_SessionManager.InitializeSessionPool  Failed\n");
		return false;
	}
	m_SessionManager.SetTimeForKickOutIdleSession(nwsp.nwspSession.usKickOutSessionTime);

	m_CloseSocketThread.SetNetWorkServices(this);
	if (!m_CloseSocketThread.Initialize(1, 1, nwsp.nwspEpollThread.unEpollThreadCount, nwsp.nwspRecvThread.usThreadCount, nwsp.nwspSendThread.unSendThreadCount, 1, nwsp.nwspCloseThread.unCloseWorkItemCount))
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Initialize", __LINE__,
														"m_CloseSocketThread.Initialize Failed\n");
		return false;
	}
	m_unCloseThreadCount = 1;

	if (!m_FixCircleBufWillCloseSessionInMainLogic.Initialize(sizeof(LSession*), nwsp.nwspSession.unCloseWorkItemCount))
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Initialize", __LINE__,
																				"m_FixCircleBufWillCloseSessionInMainLogic.Initialize Failed\n");
		return false;
	}

	m_AcceptThread.SetNetWorkServices(this);
	if (!m_AcceptThread.Initialize(1, nwsp.nwspBase.pListenIP, nwsp.nwspBase.usListenPort, nwsp.nwspBase.unListenListSize, nwsp.nwspSession.unMaxPacketLen, nwsp.nwspSession.unSendBufLen, true))
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Initialize", __LINE__,
										"m_AcceptThread.Initialize  Failed\n");
		return false;
	}
	m_unAcceptThreadCount = 1;

	if (!InitializeFixLenCircleBufOfRecvedPacket(nwsp.nwspBase.unAcceptThreadToMainThreadRecvedPacketPoolLen, nwsp.nwspRecvThread.unRecvThreadToMainThreadRecvedPacketPoolLen, nwsp.nwspCloseThread.unCloseThreadToMainThreadRecvedPacketPoolLen))
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Initialize", __LINE__,
						"InitializeFixLenCircleBufOfRecvedPacket.Init  Failed\n");
		return false;
	}

	return true;
}
void LNetWorkServices::PrintBufferInfos()
{
	InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LNetWorkServices::PrintBufferInfos", __LINE__,
							"\tNetWorkServices Print All Buffer Infos\n");
	int nCloseWorkItemCount = m_FixCircleBufWillCloseSessionInMainLogic.GetCurrentExistCount();
	int nCloseWorkMaxCount = m_FixCircleBufWillCloseSessionInMainLogic.GetMaxItemCount();
	InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LNetWorkServices::PrintBufferInfos", __LINE__,
								"\t\tCloseWorkItemCount:%d, CloseWorkMaxCount:%d, PercentInfo:%f\n", nCloseWorkItemCount, nCloseWorkMaxCount, float(nCloseWorkItemCount) / float(nCloseWorkMaxCount));

	for (unsigned int unIndex = 0; unIndex < m_unAcceptThreadCount; ++unIndex)
	{
		int nRecvedPacketCount 		= m_pArrayAcceptThreadRecvedPacket[unIndex].GetCurrentExistCount();
		int nRecvedPacketMaxCount 	= m_pArrayAcceptThreadRecvedPacket[unIndex].GetTotalBufLen();
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LNetWorkServices::PrintBufferInfos", __LINE__,
										"\t\tAcceptThreadID:%u, RecvedPacketCount:%d, RecvedPacketMaxCount:%d, PercentInfo:%f\n",
										unIndex + 1, nRecvedPacketCount, nRecvedPacketMaxCount, float(nRecvedPacketCount) / float(nRecvedPacketMaxCount));
	}

	for (unsigned int unIndex = 0; unIndex < m_unRecvThreadCount; ++unIndex)
	{
		int nRecvedPacketCount 		= m_pArrayRecvThreadRecvedPacket[unIndex].GetCurrentExistCount();
		int nRecvedPacketMaxCount 	= m_pArrayRecvThreadRecvedPacket[unIndex].GetTotalBufLen();
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LNetWorkServices::PrintBufferInfos", __LINE__,
										"\t\tRecvThreadID:%u, RecvedPacketCount:%d, RecvedPacketMaxCount:%d, PercentInfo:%f\n",
										unIndex + 1, nRecvedPacketCount, nRecvedPacketMaxCount, float(nRecvedPacketCount) / float(nRecvedPacketMaxCount));
	}

	for (unsigned int unIndex = 0; unIndex < m_unCloseThreadCount; ++unIndex)
	{
		int nRecvedPacketCount 		= m_pArrayCloseThreadRecvedPacket[unIndex].GetCurrentExistCount();
		int nRecvedPacketMaxCount 	= m_pArrayCloseThreadRecvedPacket[unIndex].GetTotalBufLen();
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_DEBUG, LOG_SUBTYPE_DATA, 0, __FILE__, "LNetWorkServices::PrintBufferInfos", __LINE__,
										"\t\tCloseThreadID:%u, RecvedPacketCount:%d, RecvedPacketMaxCount:%d, PercentInfo:%f\n",
										unIndex + 1, nRecvedPacketCount, nRecvedPacketMaxCount, float(nRecvedPacketCount) / float(nRecvedPacketMaxCount));
	}
}
//	初始化各个线程接收数据包队列
bool LNetWorkServices::InitializeFixLenCircleBufOfRecvedPacket(unsigned int unMaxAcceptThreadItemCount,
		unsigned int unMaxRecvThreadItemCount,
		unsigned int unMaxCloseThreadItemCount)
{
	if (m_unAcceptThreadCount == 0 || unMaxAcceptThreadItemCount == 0
			|| m_unRecvThreadCount == 0 || unMaxRecvThreadItemCount == 0
			|| m_unCloseThreadCount == 0 || unMaxCloseThreadItemCount == 0)
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::InitializeFixLenCircleBufOfRecvedPacket", __LINE__,
																						"m_unAcceptThreadCount == 0 || unMaxAcceptThreadItemCount == 0 \
			|| m_unRecvThreadCount == 0 || unMaxRecvThreadItemCount == 0 \
			|| m_unCloseThreadCount == 0 || unMaxCloseThreadItemCount == 0\n");
		return false;
	}
	//	初始化连接上来的队列
	m_pArrayAcceptThreadRecvedPacket = new LVarLenCircleBuf[m_unAcceptThreadCount];
	if (m_pArrayAcceptThreadRecvedPacket == NULL)
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::InitializeFixLenCircleBufOfRecvedPacket", __LINE__,
																								"m_pArrayAcceptThreadRecvedPacket == NULL\n");
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < m_unAcceptThreadCount; ++unIndex)
	{
		if (!m_pArrayAcceptThreadRecvedPacket[unIndex].InitializeVarLenCircleBuf(unMaxAcceptThreadItemCount))
		{
			InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::InitializeFixLenCircleBufOfRecvedPacket", __LINE__,
																											"m_pArrayAcceptThreadRecvedPacket[%u].Initialize Failed\n", unIndex);
			return false;
		}
	}

	m_pArrayRecvThreadRecvedPacket = new LVarLenCircleBuf[m_unRecvThreadCount];
	if (m_pArrayRecvThreadRecvedPacket == NULL)
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::InitializeFixLenCircleBufOfRecvedPacket", __LINE__,
																													"m_pArrayRecvThreadRecvedPacket == NULL\n");
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < m_unRecvThreadCount; ++unIndex)
	{
		if (!m_pArrayRecvThreadRecvedPacket[unIndex].InitializeVarLenCircleBuf(unMaxRecvThreadItemCount))
		{
			InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::InitializeFixLenCircleBufOfRecvedPacket", __LINE__,
																																"m_pArrayRecvThreadRecvedPacket[%u].Initialize\n", unIndex);
			return false;
		}
	}

	m_pArrayCloseThreadRecvedPacket = new LVarLenCircleBuf[m_unCloseThreadCount];
	if (m_pArrayCloseThreadRecvedPacket == NULL)
	{
		InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::InitializeFixLenCircleBufOfRecvedPacket", __LINE__,
																																		"m_pArrayCloseThreadRecvedPacket == NULL\n");
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < m_unCloseThreadCount; ++unIndex)
	{
		if (!m_pArrayCloseThreadRecvedPacket[unIndex].InitializeVarLenCircleBuf(unMaxCloseThreadItemCount))
		{
			InfoRecorder(LOG_TYPE_INITIALIZE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::InitializeFixLenCircleBufOfRecvedPacket", __LINE__,
																																					"m_pArrayCloseThreadRecvedPacket[%u].Initialize\n", unIndex);
			return false;
		}
	}
	return true;
}
//	加入接收的数据包
bool LNetWorkServices::AddRecvedPacketToMainLogicVarLenCircleBuf(E_Thread_Type eThreadType, unsigned int unThreadID, char* pPacketData, unsigned int unPacketDataLen, uint64_t u64SessionID)
{
	if (pPacketData == NULL || unPacketDataLen == 0)
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddRecvedPacketToMainLogicFixLenCircleBuf", __LINE__,
				"pPacketData == NULL || unPacketDataLen == 0, ThreadID:%u, PacketDataPointer:%p, PacketDataLen:%u\n", unThreadID, pPacketData, unPacketDataLen);
		return false;
	}
	if (eThreadType == E_Thread_Type_Accept)		//	AcceptThread线程中执行
	{
		if (unThreadID == 0 || unThreadID > m_unAcceptThreadCount)
		{
			InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddRecvedPacketToMainLogicFixLenCircleBuf", __LINE__,
																																								"unThreadID == 0 || unThreadID > m_unAcceptThreadCount, ThreadID:%u, AcceptThreadCount:%u\n", unThreadID, m_unAcceptThreadCount);
			return false;
		}
		E_Circle_Error eError = m_pArrayAcceptThreadRecvedPacket[unThreadID - 1].AddData(pPacketData, unPacketDataLen);
		if (eError == E_Circle_Buf_No_Error)
		{
			return true;
		}
		else
		{
			InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddRecvedPacketToMainLogicFixLenCircleBuf", __LINE__,
					"m_pArrayAcceptThreadRecvedPacket.AddData, eError != E_Circle_Buf_No_Error, ThreadID:%u, ErrorCode:%d\n", unThreadID, eError);
			return false;
		}
	}
	else if (eThreadType == E_Thread_Type_Recv)	//	RecvThread线程中执行
	{
		if (unThreadID == 0 || unThreadID > m_unRecvThreadCount)
		{
			InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddRecvedPacketToMainLogicFixLenCircleBuf", __LINE__,
					"unThreadID == 0 || unThreadID > m_unRecvThreadCount, ThreadID:%u, RecvThreadCount:%u\n", unThreadID, m_unRecvThreadCount);
			return false;
		}
		//E_Circle_Error eError = m_pArrayRecvThreadRecvedPacket[unThreadID - 1].AddData(pPacketData, unPacketDataLen);u64SessionID
		E_Circle_Error eError = m_pArrayRecvThreadRecvedPacket[unThreadID - 1].AddSessionIDAndData(u64SessionID, pPacketData, unPacketDataLen);
		if (eError == E_Circle_Buf_No_Error)
		{
			return true;
		}
		else
		{
			InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddRecvedPacketToMainLogicFixLenCircleBuf", __LINE__,
								"m_pArrayRecvThreadRecvedPacket.AddData, eError != E_Circle_Buf_No_Error, ThreadID:%u, ErrorCode:%d\n", unThreadID, eError);
			return false;
		}
	}
	else if (eThreadType == E_Thread_Type_Close)		//	CloseThread线程中执行
	{
		if (unThreadID == 0 || unThreadID > m_unCloseThreadCount)
		{
			InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddRecvedPacketToMainLogicFixLenCircleBuf", __LINE__,
								"unThreadID == 0 || unThreadID > m_unCloseThreadCount, ThreadID:%u, CloseThreadCount:%u\n", unThreadID, m_unCloseThreadCount);
			return false;
		}
		E_Circle_Error eError = m_pArrayCloseThreadRecvedPacket[unThreadID - 1].AddData(pPacketData, unPacketDataLen);
		if (eError == E_Circle_Buf_No_Error)
		{
			return true;
		}
		else
		{
			InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddRecvedPacketToMainLogicFixLenCircleBuf", __LINE__,
											"m_pArrayCloseThreadRecvedPacket.AddData eError != E_Circle_Buf_No_Error, ThreadID:%u, ErrorCode:%d\n", unThreadID, eError);
			return false;
		}
	}
	else
	{
		return false;
	}
}

int LNetWorkServices::ProcessRecvedPacket(unsigned int unMaxProcessRecvedPakcetNumPerQueue)
{
	m_nProcessedThisRound = 0;
	//	处理连接需要断开事件
	m_nProcessedThisRound += ProcessWillCloseSessionInMainLogic();

	for (unsigned int unIndex = 0; unIndex < m_unAcceptThreadCount; ++unIndex)
	{
		m_nProcessedThisRound += ProcessRecvedPacketFromAcceptThread(&m_pArrayAcceptThreadRecvedPacket[unIndex], unMaxProcessRecvedPakcetNumPerQueue);
	}

	for (unsigned int unIndex = 0; unIndex < m_unRecvThreadCount; ++unIndex)
	{
		m_nProcessedThisRound += ProcessRecvedPacketFromRecvThread(&m_pArrayRecvThreadRecvedPacket[unIndex], unMaxProcessRecvedPakcetNumPerQueue);
	}

	for (unsigned int unIndex = 0; unIndex < m_unCloseThreadCount; ++unIndex)
	{
		m_nProcessedThisRound += ProcessRecvedPacketFromCloseThread(&m_pArrayCloseThreadRecvedPacket[unIndex], unMaxProcessRecvedPakcetNumPerQueue);
	}
#if PRINT_INFO_TO_DEBUG
	time_t tNow = time(NULL);
	if (tNow - m_tLastPrintInfosTime > 80)
	{
		PrintBufferInfos();
		m_tLastPrintInfosTime = tNow;
	}
#endif
	return m_nProcessedThisRound;
}
int LNetWorkServices::GetProcessedCountThisRound()
{
	return m_nProcessedThisRound;
}
int LNetWorkServices::ProcessRecvedPacketFromAcceptThread(LVarLenCircleBuf* pVarLenCircleBuf, unsigned int unProcessedNum)
{
	int nProcessedNumThisFunction = 0;
	while (1)
	{
		m_unTempAcceptedSessionInfoLen = 0;
		E_Circle_Error error = pVarLenCircleBuf->GetData(m_szTempAcceptedSessionInfo, MAX_DATA_BUF_LEN_TO_WRITE_ACCEPTED_SESSION_INFO, m_unTempAcceptedSessionInfoLen);
		if (error == E_Circle_Buf_No_Error)
		{
			if (m_unTempAcceptedSessionInfoLen < sizeof(uint64_t) + 20 + sizeof(unsigned short) + 2 * sizeof(int))
			{
				InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::ProcessRecvedPacketFromAcceptThread", __LINE__,
															"m_unTempAcceptedSessionInfoLen ErrorLen, GetttedLen:%u, NeedLen:%d\n", m_unTempAcceptedSessionInfoLen, sizeof(uint64_t) + 20 + sizeof(unsigned short) + 2 * sizeof(int));
				continue;
			}
			int nDataOffSet = 0;

			uint64_t u64SessionID = *((uint64_t*)(m_szTempAcceptedSessionInfo + nDataOffSet));
			nDataOffSet += sizeof(uint64_t);

			char szRemoteIPName[20]; memset(szRemoteIPName, 0, sizeof(szRemoteIPName));
			memmove(szRemoteIPName, m_szTempAcceptedSessionInfo + nDataOffSet, 20);
			nDataOffSet += 20;

			unsigned short usRemotePort = *((unsigned short*)(m_szTempAcceptedSessionInfo + nDataOffSet));
			nDataOffSet += sizeof(unsigned short);

			int nRecvThreadID = *((int*)(m_szTempAcceptedSessionInfo + nDataOffSet));
			nDataOffSet += sizeof(int);

			int nSendThreadID = *((int*)(m_szTempAcceptedSessionInfo + nDataOffSet));
			nDataOffSet += sizeof(int);

			if (m_pServerBaseNetWork != NULL)
			{
				m_pServerBaseNetWork->OnAddSession(u64SessionID, szRemoteIPName, usRemotePort, nRecvThreadID, nSendThreadID);
			}
			//	已经处理了Accept，主线程知道了存在这个连接，那么开始接受数据
//			LSession* pSession = m_SessionManager.FindSession(u64SessionID);
//			if (pSession != NULL)
//			{
//				if (pSession->GetCloseWorkSendedToCloseThread() == 0)		//	如果其他原因需要关闭，那么不用投送EPOLLIN事件了
//				{
//					m_EpollThreadManager.PostEpollReadEvent(pSession);
//				}
//			}
			if (!AddFirstRecvOfSession(u64SessionID))
			{
				InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::ProcessRecvedPacketFromAcceptThread", __LINE__,
																			"AddFirstRecvOfSession Failed\n");
			}
			nProcessedNumThisFunction++;
		}
		else
		{
			break;
		}
		if (nProcessedNumThisFunction >= unProcessedNum)
		{
			break;
		}
	}
	return nProcessedNumThisFunction;
}
int LNetWorkServices::ProcessRecvedPacketFromRecvThread(LVarLenCircleBuf* pVarLenCircleBuf, unsigned int unProcessedNum)
{
	int nProcessedNumThisFunction = 0;

	while (1)
	{
		m_unTempPacketDataLen = 0;
		E_Circle_Error error = pVarLenCircleBuf->GetData(m_szTempPacketData, MAX_TEMP_PACKET_BUF_LEN, m_unTempPacketDataLen);
		if (error == E_Circle_Buf_No_Error)
		{
			if (m_unTempPacketDataLen < sizeof(uint64_t))
			{
				InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::ProcessRecvedPacketFromRecvThread", __LINE__,
																							"m_unTempPacketDataLen ErrorLen, GetttedLen:%u, LeastNeedLen:%d\n", m_unTempPacketDataLen, sizeof(uint64_t));
				continue;
			}
			uint64_t u64SessionID = *((uint64_t*)m_szTempPacketData);
			if (m_pServerBaseNetWork != NULL)
			{
				m_pServerBaseNetWork->OnRecvedPacket(u64SessionID, m_szTempPacketData + sizeof(uint64_t), m_unTempPacketDataLen - sizeof(uint64_t));
			}
			nProcessedNumThisFunction++;
		}
		else
		{
			break;
		}
		if (nProcessedNumThisFunction >= unProcessedNum)
		{
			break;
		}
	}
	return nProcessedNumThisFunction;
}
int LNetWorkServices::ProcessRecvedPacketFromCloseThread(LVarLenCircleBuf* pVarLenCircleBuf, unsigned int unProcessedNum)
{
	int nProcessedNumThisFunction = 0;

	while (1)
	{
		m_unTempClosePacketDataLen = 0;
		E_Circle_Error error = pVarLenCircleBuf->GetData(m_szTempClosePacketData, 32, m_unTempClosePacketDataLen);
		if (error == E_Circle_Buf_No_Error)
		{
			if (m_unTempClosePacketDataLen != sizeof(uint64_t))
			{
				InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::ProcessRecvedPacketFromCloseThread", __LINE__,
																			"m_unTempClosePacketDataLen ErrorLen, GetttedLen:%u, NeedLen:%d\n", m_unTempClosePacketDataLen, sizeof(uint64_t));
				continue;
			}
			uint64_t u64SessionID = *((uint64_t*)m_szTempClosePacketData);
			if (m_pServerBaseNetWork != NULL)
			{
				m_pServerBaseNetWork->OnRemoveSession(u64SessionID);
			}
			nProcessedNumThisFunction++;
		}
		else
		{
			break;
		}
		if (nProcessedNumThisFunction >= unProcessedNum)
		{
			break;
		}
	}
	return nProcessedNumThisFunction;
}

bool LNetWorkServices::Start()
{
	if (!m_RecvThreadManager.StartAllRecvThread())
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Start", __LINE__,
													"m_RecvThreadManager.StartAllRecvThread Failed\n");
		return false;
	}
	if (!m_SendThreadManager.StartAllSendThread())
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Start", __LINE__,
															"m_SendThreadManager.StartAllSendThread Failed\n");
		return false;
	}
	if (!m_EpollThreadManager.StartAllEpollThread())
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Start", __LINE__,
															"m_EpollThreadManager.StartAllEpollThread Failed\n");
		return false;
	}
	if (!m_CloseSocketThread.Start())
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Start", __LINE__,
															"m_CloseSocketThread.Start Failed\n");
		return false;
	}

	if (!m_AcceptThread.Start())
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::Start", __LINE__,
															"m_AcceptThread.Start Failed\n");
		return false;
	}

	return true;
}

void LNetWorkServices::Stop()
{
	m_AcceptThread.StopAcceptThread();

	m_EpollThreadManager.StopAllEpollThread();
	m_SendThreadManager.StopAllSendThread();
	m_RecvThreadManager.StopAllRecvThread();
	m_CloseSocketThread.StopCloseSocketThread();
}

bool LNetWorkServices::SendPacket(uint64_t u64SessionID, char* pData, unsigned int unDataLen)
{
	if (pData == NULL || unDataLen == 0)
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::SendPacket", __LINE__,
																					"pData == NULL || unDataLen == 0, SessionID:%llu, Pointer:%p, DataLen:%u\n", u64SessionID, pData, unDataLen);
		return false;
	}
	LSession* pSession = GetSessionManager().FindSession(u64SessionID);
	if (pSession == NULL)
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::SendPacket", __LINE__,
																							"pSession == NULL, SessionID:%llu, Pointer:%p, DataLen:%u\n", u64SessionID, pData, unDataLen);
		return false;
	}
	int nSendThreadID = pSession->GetSendThreadID();
	//	关闭连接的工作已经向关闭线程发送，那么这里就不再发送数据了，等待连接的关闭
	if (pSession->GetCloseWorkSendedToCloseThread() != 0)
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::SendPacket", __LINE__,
																									"pSession->GetCloseWorkSendedToCloseThread(), SessionID:%llu, Pointer:%p, DataLen:%u\n", u64SessionID, pData, unDataLen);
		return false;
	}
	if (!pSession->AppendDataToSend(pData, unDataLen))
	{
		KickOutSession(u64SessionID);

		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::SendPacket", __LINE__,
																			"pSession->AppendDataToSend Failed, ThreadID:%d, SendThreadCount:%u\n", nSendThreadID, m_unSendThreadCount);
		return false;
	}

	LSendThread* pSendThread = GetSendThreadManager().GetSendThread(nSendThreadID);
	if (pSendThread != NULL)
	{
		bool bAddSendSuccess = pSendThread->AddOneSendWorkItem(u64SessionID);
		if (!bAddSendSuccess)
		{
			InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::SendPacket", __LINE__,
																						"pSendThread->AddOneSendWorkItem Failed, ThreadID:%d, SendThreadCount:%u, SessionID:%llu\n", nSendThreadID, m_unSendThreadCount, u64SessionID);
			KickOutSession(u64SessionID);
			return false;
		}
	}
	else
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::SendPacket", __LINE__,
																					"pSendThread == NULL, ThreadID:%d, SendThreadCount:%u, SessionID:%llu\n", nSendThreadID, m_unSendThreadCount, u64SessionID);

		KickOutSession(u64SessionID);
		return false;
	}
	return true;
}


//	删除一段时间内没有通讯的连接
void LNetWorkServices::KickOutIdleSession()
{ 
	m_SessionManager.KickOutIdleSession();
}

bool LNetWorkServices::GetSessionIPAndPort(unsigned long long u64SessionID, char* pszBuf, unsigned short usbufLen)
{
	return m_SessionManager.GetSessionIPAndPort(u64SessionID, pszBuf, usbufLen);
}


void LNetWorkServices::KickOutSession(uint64_t u64SessionID)
{
	ProcessKickOutSession(u64SessionID);
}

//	释放占用的资源
void LNetWorkServices::ReleaseNetWorkServicesResource()
{
	m_RecvThreadManager.ReleaseRecvThreadManagerResource();
	m_SendThreadManager.ReleaseSendThreadManagerResource();
	m_EpollThreadManager.ReleaseEpollThreadManagerResource();
	m_AcceptThread.ReleaseAcceptThreadResource();
	m_CloseSocketThread.ReleaseCloseSocketThreadResource();

	m_SessionManager.ReleaseMasterSessionManagerResource();
}


void LNetWorkServices::GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort)
{
	m_AcceptThread.GetListenIpAndPort(pBuf, unBufSize, usPort);
}


void LNetWorkServices::AddWillCloseSessionInMainLogic(LSession* pSession)
{
	E_Circle_Error eError = m_FixCircleBufWillCloseSessionInMainLogic.AddItems((char*)&pSession, 1);
	if (eError != E_Circle_Buf_No_Error)
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddWillCloseSessionInMainLogic", __LINE__,
																			"eError != E_Circle_Buf_No_Error, ErrorCode:%d\n", eError);
	}
}
bool LNetWorkServices::GetOneWillCloseSessionInMainLogic(LSession** pSession)
{
	E_Circle_Error eError = m_FixCircleBufWillCloseSessionInMainLogic.GetOneItem((char*)(&(*pSession)), sizeof(LSession*));
	if (eError != E_Circle_Buf_No_Error && eError != E_Circle_Buf_Is_Empty)
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::GetOneWillCloseSessionInMainLogic", __LINE__,
																					"eError != E_Circle_Buf_No_Error && eError != E_Circle_Buf_Is_Empty, ErrorCode:%d\n", eError);
		return false;
	}
	if (eError == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}
void LNetWorkServices::ProcessKickOutSession(uint64_t u64SessionID)
{
	LSession* pSession = GetSessionManager().FindSession(u64SessionID);
	if (pSession == NULL)
	{
		return ;
	}
	if (pSession->GetCloseWorkSendedToCloseThread() == 0)
	{
		//	发送一个关闭工作给关闭线程,所有连接相关的线程都不再操作连接了，那么就可以关闭该连接了
		t_Client_Need_To_Close cntc;
		cntc.u64SessionID 						= u64SessionID;

		m_CloseSocketThread.AppendToClose(E_Thread_Type_MainLogic, 1, cntc);

		pSession->SetCloseWorkSendedToCloseThread(1);
	}
}

//	主线程必须调用这个函数，来这却处理关闭连接
int LNetWorkServices::ProcessWillCloseSessionInMainLogic()
{
	int nProcessedCount = 0;
	LSession* pSession = NULL;
	while (GetOneWillCloseSessionInMainLogic(&pSession))
	{
		if (pSession != NULL)
		{
			pSession->SetMainLogicThreadStopProcess(1);

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
				GetSessionManager().MoveWillCloseSessionToSessionPool(pSession);
			}
		}
		nProcessedCount++;
		if (nProcessedCount >= 100)
		{
			break;
		}
	}
	return nProcessedCount;
}

//	这个函数被主线程调用
bool LNetWorkServices::AddFirstRecvOfSession(uint64_t u64SessionID)
{
	LSession* pSession = m_SessionManager.FindSession(u64SessionID);
	if (pSession == NULL)
	{
		return false;
	}
	int nRecvThreadID = pSession->GetRecvThreadID();
	LRecvThread* pRecvThread = m_RecvThreadManager.GetRecvThread((unsigned short)nRecvThreadID);
	if (pRecvThread == NULL)
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddFirstRecvOfSession", __LINE__,
																							"m_RecvThreadManager.GetRecvThread Failed, SessionID:%llu, RecvthreadID:%d\n", u64SessionID, nRecvThreadID);
		return false;
	}
	if (!pRecvThread->AddFirstRecvOfSession(u64SessionID))
	{
		InfoRecorder(LOG_TYPE_NETWORK_SERVICE, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LNetWorkServices::AddFirstRecvOfSession", __LINE__,
																									"AddFirstRecvOfSession Failed, SessionID:%llu, RecvthreadID:%d\n", u64SessionID, nRecvThreadID);
		return false;
	}
	return true;
}
void LNetWorkServices::PrintManagerInfos()
{
	m_EpollThreadManager.PrintEpollThreadStatus();
	m_RecvThreadManager.PrintSendThreadRefStatus();
	m_SendThreadManager.PrintSendThreadRefStatus();
}
void LNetWorkServices::SetLogTypeAndLevel(unsigned int unLogType, unsigned int unLevel)
{
	if (m_pInfoRecorderBase == NULL)
	{
		return ;
	}
	m_pInfoRecorderBase->SetLogType(unLogType);
	m_pInfoRecorderBase->SetLogLevel(unLevel);
}
void LNetWorkServices::GetLogTypeAndLevel(unsigned int& unLogType, unsigned int& unLevel)
{
	if (m_pInfoRecorderBase == NULL)
	{
		return ;
	}
	unLogType	= m_pInfoRecorderBase->GetLogType();
	unLevel		= m_pInfoRecorderBase->GetLogLevel();
}

void LNetWorkServices::SetInfoRecorderBase(LInfoRecorderBase* pInfoRecorderBase)
{
	m_pInfoRecorderBase = pInfoRecorderBase;
}
void LNetWorkServices::InfoRecorder(unsigned int unLogType, unsigned int unLevel, unsigned int unLogSubType, unsigned int unThreadID, const char* pszFileName, const char* pszFunctionName, int nLine, const char* pszFmt, ...)
{
	if (m_pInfoRecorderBase == NULL)
	{
		return ;
	}
	va_list ap;
	va_start(ap, pszFmt);
	m_pInfoRecorderBase->InfoRecorder(unLogType, unLevel, unLogSubType, unThreadID, pszFileName, pszFunctionName, nLine, pszFmt, ap);
	va_end(ap);
}

