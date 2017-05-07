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

#include "LSessionManager.h"
#include "LCloseSocketThread.h"
#include "LNetWorkServices.h"
#include "LAutoReleaseMutex.h"


//	================================MasterSessionManager=============started
LMasterSessionManager::LMasterSessionManager()
{
	pthread_mutex_init(&m_mutexForDirty, NULL);
	m_pNetWorkServices				= NULL;
	m_usTime 							= 30;		//	20秒未通信就踢出连接，默认
	m_nCurrentOnlineSessionNum		= 0;
	pthread_mutex_init(&m_MutexProtectedForWillReuseManager, NULL);

	m_pArraySessionManagerSlotInfo	= NULL;
	m_unSlotNum 							= 0;
}
LMasterSessionManager::~LMasterSessionManager()
{
	pthread_mutex_destroy(&m_mutexForDirty);
	pthread_mutex_destroy(&m_MutexProtectedForWillReuseManager);
}

//	public member
bool LMasterSessionManager::InitializeSessionPool(unsigned int unMaxClientNum, unsigned int unRecvBufSizeLen, unsigned int unSendBufSizeLen)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	
	if (unMaxClientNum == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::InitializeSessionPool", __LINE__,
																	"unMaxClientNum == 0\n");
		return false;
	}
	m_unSlotNum = unMaxClientNum;


	m_pArraySessionManagerSlotInfo = new t_Session_Manager_Slot_Info[m_unSlotNum];
	if (m_pArraySessionManagerSlotInfo == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::InitializeSessionPool", __LINE__,
																			"m_pArraySessionManagerSlotInfo == NULL, SlotNum:%u\n", m_unSlotNum);
		return false;
	}
	for (int i = 0; i < m_unSlotNum; ++i)
	{
		m_pArraySessionManagerSlotInfo[i].nSlotIndex 	= i;
		m_pArraySessionManagerSlotInfo[i].nSessionIndex = 1;	//	所有的ID从1开始
		m_SessionManagerSlotFree.push(i);
	}

	for (unsigned int uIndex = 0; uIndex < m_unSlotNum; ++uIndex)
	{
		LSession* pTemp = new LSession; 
		if (pTemp == NULL)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::InitializeSessionPool", __LINE__,
																						"pTemp == NULL, SlotNum:%u, Index:%u\n", m_unSlotNum, uIndex);
			return false;
		}
		if (!pTemp->InitializeRecvAndSendBuf(unRecvBufSizeLen, unSendBufSizeLen))
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::InitializeSessionPool", __LINE__,
																									"pTemp == NULL, SlotNum:%u, Index:%u, RecvBufLen:%u, SendBufLen:%u\n", m_unSlotNum, uIndex, unRecvBufSizeLen, unSendBufSizeLen);
			return false;
		}
		pTemp->Reset();

		m_queueDirtySessionManager.push(pTemp);
	}

	if (!m_cFixLenCircleBufNewSessionOnline.Initialize(sizeof(uint64_t), unMaxClientNum * 8 + 100 * 8))
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::InitializeSessionPool", __LINE__,
														"m_cFixLenCircleBufNewSessionOnline.Initialize Failed, MaxClientNum:%u\n", unMaxClientNum);
		return false;
	}
	return true;
}

//	AcceptThread线程调用函数
//	Session一旦分配出去，统一经过CloseThread进行回收
LSession* LMasterSessionManager::AllocSession(int nSessionSocket)
{
	LSession* pSession = NULL;
	LAutoReleaseMutex AutoRelease(&m_mutexForDirty);

	if (m_SessionManagerSlotFree.empty())
	{
		AutoRelease.Free();
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::AllocSession", __LINE__,
																"m_SessionManagerSlotFree.empty\n");
		return NULL;
	}
	int nSlotIndex = m_SessionManagerSlotFree.front();
	m_SessionManagerSlotFree.pop();

	if (nSlotIndex >= m_unSlotNum)		//  这里取出的SlotIndex不可能大于总数，如果大于，那么就是错误的
	{
		AutoRelease.Free();
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::AllocSession", __LINE__,
																		"nSlotIndex >= m_unSlotNum, SlotIndex:%d, SlotNum:%u\n", nSlotIndex, m_unSlotNum);
		return NULL;
	}
	if (m_queueDirtySessionManager.empty())
	{
		AutoRelease.Free();

		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::AllocSession", __LINE__,
																				"m_queueDirtySessionManager.empty\n");
		//	没有空余的Session了，把Slot放回空闲队列中
		m_SessionManagerSlotFree.push(nSlotIndex);
		return NULL;
	}
	pSession = m_queueDirtySessionManager.front();
	m_queueDirtySessionManager.pop();

	pSession->Reset();

	uint64_t u64SessionID = 0;
	u64SessionID = nSlotIndex;
	u64SessionID <<= 32;

	int nSessionIndex = m_pArraySessionManagerSlotInfo[nSlotIndex].nSessionIndex;
	u64SessionID |= nSessionIndex;

	pSession->SetSessionID(u64SessionID);

	pSession->SetSocket(nSessionSocket);
	pSession->SetSessionConnecttedTime();
	pSession->SetEpollBindParam(pSession->GetSessionID(), nSessionSocket);

	m_pArraySessionManagerSlotInfo[nSlotIndex].pSession = pSession;
	AutoRelease.Free();

	//	如果这里有多个AcceptThreand线程，那么这里要加锁，现在只有一个线程，不用加锁
	E_Circle_Error eErrorCode = m_cFixLenCircleBufNewSessionOnline.AddItems((char*)&u64SessionID, 1);
	if (eErrorCode != E_Circle_Buf_No_Error)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::AllocSession", __LINE__,
																						"m_cFixLenCircleBufNewSessionOnline.AddItems Failed, ErrorCode:%d\n", eErrorCode);
	}
	return pSession;
}

//	任何线程都可能调用该函数
LSession* LMasterSessionManager::FindSession(uint64_t u64SessionID)
{ 
	int nSlotIndex = (u64SessionID >> 32) & 0xFFFFFFFF;
	if (nSlotIndex < 0 || nSlotIndex >= m_unSlotNum)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::FindSession", __LINE__,
													"SlotIndex:%d, SlotNum:%u\n", nSlotIndex, m_unSlotNum);
		return NULL;
	}
	int nSessionIndex = u64SessionID & 0xFFFFFFFF;

	if (nSessionIndex != m_pArraySessionManagerSlotInfo[nSlotIndex].nSessionIndex)
	{
		return NULL;
	}
	return m_pArraySessionManagerSlotInfo[nSlotIndex].pSession;
}

//	关闭线程调用该函数
void LMasterSessionManager::FreeSession(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}
	unsigned long long u64SessionID = pSession->GetSessionID();
	int nSlotIndex = (u64SessionID >> 32) & 0xFFFFFFFF;
	if (nSlotIndex < 0 || nSlotIndex >= m_unSlotNum)
	{
		//	致命错误,记录日志 fatal error
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::FreeSession", __LINE__,
															"SlotIndex:%d, SlotNum:%u\n", nSlotIndex, m_unSlotNum);
		return ;
	}
	//	将sessionIndex增加1，其他线程访问时，不能再找出该Session
	__sync_fetch_and_add(&m_pArraySessionManagerSlotInfo[nSlotIndex].nSessionIndex, 1);

	if (pSession->GetSocket() != -1)
	{
		close(pSession->GetSocket()); 
	}
}

void LMasterSessionManager::Release()
{

}
// 任何线程可能调用，在Session的所有的使用线程都放弃使用Session时，被其中的某个使用线程调用
void LMasterSessionManager::FreeSessionToPool(LSession* pSession)
{
	if (pSession == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::FreeSessionToPool", __LINE__,
																			"pSession == NULL\n");
		return ;
	}
	unsigned long long u64SessionID = pSession->GetSessionID();
	int nSlotIndex = (u64SessionID >> 32) & 0xFFFFFFFF;
	if (nSlotIndex < 0 || nSlotIndex >= m_unSlotNum)
	{
		//	致命错误,记录日志 fatal error
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::FreeSessionToPool", __LINE__,
																	"SlotIndex:%d, SlotNum:%u\n", nSlotIndex, m_unSlotNum);
		return ;
	}

	LAutoReleaseMutex AutoRelease(&m_mutexForDirty);
	m_queueDirtySessionManager.push(pSession);	
	m_SessionManagerSlotFree.push(nSlotIndex);
	m_pArraySessionManagerSlotInfo[nSlotIndex].pSession = NULL;
	AutoRelease.Free();
}


//	关闭一定时间内没有通讯的连接,死连接,
//	这个函数在主线程调用
void LMasterSessionManager::KickOutIdleSession()
{ 
	//	将sessionID加入到list，进行检查
	uint64_t u64SessionID = 0;
	while (m_cFixLenCircleBufNewSessionOnline.GetOneItem((char*)&u64SessionID, sizeof(uint64_t)) == E_Circle_Buf_No_Error)
	{
		m_listCurrentOnlineClient.push_back(u64SessionID);
	}

	time_t tNow = time(NULL);
	list<uint64_t>::iterator _ito = m_listCurrentOnlineClient.begin();
	while (_ito != m_listCurrentOnlineClient.end())
	{
		uint64_t ullSessionID = *_ito;
		LSession* pSession = FindSession(ullSessionID);
		if (pSession == NULL)
		{
			_ito = m_listCurrentOnlineClient.erase(_ito);
			continue;
		}
		//	检查是否超时没有数据包
		time_t tConnectTime =  pSession->GetSessionConnecttedTime();
		int nDifTime = tNow - tConnectTime;
		int nDifRecvTime = pSession->GetLastRecvTime();
		//if (nDifTime >= nDifSendTime + 5 * 60 && nDifTime >= nDifRecvTime + 5 * 60)
		if (nDifTime > nDifRecvTime + m_usTime)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::KickOutIdleSession", __LINE__,
																				"SessionID:%llu, TimeNow:%d, TimeConnect:%d, DifRecvTime:%d, KickOutTime:%hu\n",
																				ullSessionID, time(NULL), tConnectTime, nDifRecvTime, m_usTime);
			m_pNetWorkServices->ProcessKickOutSession(pSession->GetSessionID());
			_ito = m_listCurrentOnlineClient.erase(_ito);
			continue;
		}

		_ito++;
	}

}

//	主线程调用
bool LMasterSessionManager::GetSessionIPAndPort(uint64_t u64SessionID, char* pszBuf, unsigned short usbufLen)
{ 
	LSession* pSession = FindSession(u64SessionID);
	if (pSession == NULL)
	{
		return false;
	}
	return pSession->GetIpAndPort(pszBuf, usbufLen);
}

//	释放所有的Session和SessionManager，以及它们占用的资源
void LMasterSessionManager::ReleaseMasterSessionManagerResource()
{
	while(!m_queueDirtySessionManager.empty())
	{
		LSession* pSession = m_queueDirtySessionManager.front();
		m_queueDirtySessionManager.pop();
		pSession->ReleaseSessionResource();
		delete pSession;
	}
	for (unsigned int uIndex = 0; uIndex < m_unSlotNum; ++uIndex)
	{
		if (m_pArraySessionManagerSlotInfo[uIndex].pSession != NULL)
		{
			m_pArraySessionManagerSlotInfo[uIndex].pSession->ReleaseSessionResource();
			delete m_pArraySessionManagerSlotInfo[uIndex].pSession;
			m_pArraySessionManagerSlotInfo[uIndex].pSession = NULL;
		}
	}
	if (m_pArraySessionManagerSlotInfo != NULL)
	{
		delete[] m_pArraySessionManagerSlotInfo;
		m_pArraySessionManagerSlotInfo = NULL;
	}
}

void LMasterSessionManager::AddToWillReuseManager(LSession* pSession)
{
	LAutoReleaseMutex AutoReleaseMutex(&m_MutexProtectedForWillReuseManager);
	map<LSession*, LSession*>::iterator _ito = m_mapSessionWillReuseManager.find(pSession);
	if (_ito == m_mapSessionWillReuseManager.end())
	{
		m_mapSessionWillReuseManager[pSession] = pSession;
	}
}

size_t LMasterSessionManager::GetWillReuseSessionCount()
{
	LAutoReleaseMutex AutoReleaseMutex(&m_MutexProtectedForWillReuseManager);
	return m_mapSessionWillReuseManager.size();
}
void LMasterSessionManager::MoveWillCloseSessionToSessionPool(LSession* pSession)
{
#if PRINT_INFO_TO_DEBUG
	m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::MoveWillCloseSessionToSessionPool", __LINE__,
			"Packet Received Bytes:%d, Packet Received:%d, Packet Processed:%d, PacketSendInMainLogic:%d, PacketSendedInSendThread:%d, MaxDataLenInSendBuf:%d\n",
			pSession->GetPacketRecvedBytes(), pSession->GetPacketRecved(), pSession->GetPacketProcessed(), pSession->GetPacketAllSendBytes(), pSession->GetPacketAllSendedBytes(), pSession->GetMaxDataLenInSendBuf());
#endif
	LAutoReleaseMutex AutoReleaseMutex(&m_MutexProtectedForWillReuseManager);
	map<LSession*, LSession*>::iterator _ito = m_mapSessionWillReuseManager.find(pSession);
	if (_ito != m_mapSessionWillReuseManager.end())
	{

		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::MoveWillCloseSessionToSessionPool", __LINE__,
																						"SessionID:%llu, EpollThreaID:%d, RecvThreadID:%d, SendThreadID:%d\n",
																						pSession->GetSessionID(), pSession->GetEpollThreadID(), pSession->GetRecvThreadID(), pSession->GetSendThreadID());
		m_pNetWorkServices->GetEpollThreadManager().UnBindEpollThread(pSession);
		m_pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(pSession);
		m_pNetWorkServices->GetSendThreadManager().UnBindSendThread(pSession);



		FreeSessionToPool(pSession);
		m_mapSessionWillReuseManager.erase(_ito);
	}
	else
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_DEBUG, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LMasterSessionManager::MoveWillCloseSessionToSessionPool", __LINE__, "Double Free!!\n");
	}
}

//	主线程调用
void LMasterSessionManager::PrintAllSessionInfos()
{
	pthread_mutex_lock(&m_mutexForDirty);
	int nDirtySessionCount = (int)m_queueDirtySessionManager.size();
	pthread_mutex_unlock(&m_mutexForDirty);

	printf("SessionCount:%d, DirtyCount:%d\n", m_listCurrentOnlineClient.size(), nDirtySessionCount);
}

