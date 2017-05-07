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

#include "LRecvThreadManager.h"
#include "LSession.h"
#include "LNetWorkServices.h" 


LRecvThreadManager::LRecvThreadManager()
{
	m_parrRecvDesc = NULL;
	m_usThreadCount = 0;	
	m_pNetWorkServices = NULL;
}
LRecvThreadManager::~LRecvThreadManager()
{
}

bool LRecvThreadManager::StartAllRecvThread()
{
	if (m_parrRecvDesc == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::StartAllRecvThread", __LINE__,
															"m_parrRecvDesc == NULL\n");
		return false;
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		if (!(&m_parrRecvDesc[usIndex])->pRecvThread->Start())
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::StartAllRecvThread", __LINE__,
																		"m_parrRecvDesc[%hu])->pRecvThread->Start Failed\n", usIndex);
			return false;
		}
	}
	return true;
}
bool LRecvThreadManager::StopAllRecvThread()
{
	if (m_parrRecvDesc == NULL)
	{
		return true;
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		if ((&m_parrRecvDesc[usIndex])->pRecvThread != NULL)
		{
			(&m_parrRecvDesc[usIndex])->pRecvThread->Stop(); 
		}
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		if ((&m_parrRecvDesc[usIndex])->pRecvThread != NULL)
		{
			pthread_t pID =(&m_parrRecvDesc[usIndex])->pRecvThread->GetThreadHandle(); 
			if (pID != 0)
			{
				int nJoinRes = pthread_join(pID, NULL); 
				if (nJoinRes != 0)
				{
					m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::StopAllRecvThread", __LINE__,
																							"pthread_join Failed, SystemErrorCode:%d\n", errno);
				}
			} 
		}
	} 
	return true;
}

//	unEpollThreadCount  EpollThread线程数量
//	usThreadCount 接收线程的数量
//	unRecvWorkItemCount epollin事件环形队列的最大队列数，大于该数量，那么EPOLLIN事件无法放入接收线程处理
//	unCloseWorkItemCount 处理关闭连接的缓存队列长度
bool LRecvThreadManager::Init(unsigned int unEpollThreadCount, unsigned short usThreadCount, unsigned int unRecvWorkItemCount, unsigned int unCloseWorkItemCount)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	if (usThreadCount == 0)
	{
		usThreadCount = 1;
	}
	m_usThreadCount = usThreadCount;

	m_parrRecvDesc = new t_Recv_Thread_Desc[usThreadCount];
	if (m_parrRecvDesc == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::Init", __LINE__,
													"m_parrRecvDesc == NULL, RecvThreadCount:%u\n", m_usThreadCount);
		return false;
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		(&m_parrRecvDesc[usIndex])->pRecvThread = new LRecvThread;
		if ((&m_parrRecvDesc[usIndex])->pRecvThread == NULL)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::Init", __LINE__,
																"m_parrRecvDesc[%hu])->pRecvThread == NULL\n", usIndex);
			return false;
		}
		(&m_parrRecvDesc[usIndex])->pRecvThread->SetNetServices(m_pNetWorkServices);
		m_parrRecvDesc[usIndex].pRecvThread->m_nThreadID = usIndex + 1;
		(&m_parrRecvDesc[usIndex])->usThreadID = usIndex;
		if (!(&m_parrRecvDesc[usIndex])->pRecvThread->Initialize(unEpollThreadCount, unRecvWorkItemCount, unCloseWorkItemCount))
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::Init", __LINE__,
																			"m_parrRecvDesc[%hu])->pRecvThread->Initialize Failed\n", usIndex);
			return false;
		}
	}
	return true; 
}
bool LRecvThreadManager::BindRecvThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::BindRecvThread", __LINE__,
																									"pSession == NULL\n");
		return false;
	}
	int nSelectedThreadID = SelectRecvThread();
	nSelectedThreadID += 1;			//	所有线程的id从1开始
	if (nSelectedThreadID <= 0 || nSelectedThreadID > m_usThreadCount)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::BindRecvThread", __LINE__,
																											"ThreadID:%d, RecvThreadCount:%hu\n", nSelectedThreadID, m_usThreadCount);
		return false;
	}
	pSession->SetRecvThreadID(nSelectedThreadID);
	__sync_add_and_fetch(&m_parrRecvDesc[nSelectedThreadID - 1].nBindSessionCount, 1);
	return true;
}
bool LRecvThreadManager::UnBindRecvThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::UnBindRecvThread", __LINE__,
																											"pSession == NULL\n");
		return false;
	}
	int nRecvThreadID = pSession->GetRecvThreadID();
	if (nRecvThreadID > m_usThreadCount || nRecvThreadID <= 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LRecvThreadManager::UnBindRecvThread", __LINE__,
																													"SessionRecvThreadID:%d, RecvThreadCount:%hu\n", nRecvThreadID, m_usThreadCount);
		return false;
	}
	__sync_sub_and_fetch(&m_parrRecvDesc[nRecvThreadID - 1].nBindSessionCount, 1);
	pSession->SetRecvThreadID(-1);
	return true;
}

int LRecvThreadManager::SelectRecvThread()
{
	int nThreadMinBindSessionCount = 0x0fffffff;
	int nSelectThread = -1;
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		int nRefCount = m_parrRecvDesc[usIndex].nBindSessionCount;
		if (nRefCount < nThreadMinBindSessionCount)
		{
			nThreadMinBindSessionCount = nRefCount;
			nSelectThread = usIndex;
		}
	}
	return nSelectThread;
}
LRecvThread* LRecvThreadManager::GetRecvThread(unsigned short usThreadID)
{
	if (usThreadID > m_usThreadCount || usThreadID == 0)
	{
		return NULL;
	}
	return m_parrRecvDesc[usThreadID - 1].pRecvThread;
}
void LRecvThreadManager::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
{
	m_pNetWorkServices = pNetWorkServices;
}


void LRecvThreadManager::PrintSendThreadRefStatus()
{
	printf("\tRecvThreadManager, Thread RefInfo\n");
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		int nRefCount = m_parrRecvDesc[usIndex].nBindSessionCount;
		int nFPS = m_parrRecvDesc[usIndex].pRecvThread->GetFPS();
		int nMinFPS = m_parrRecvDesc[usIndex].pRecvThread->GetMinFPS();
		int nMaxFPS = m_parrRecvDesc[usIndex].pRecvThread->GetMaxFPS();
		printf("\t\tRecvThreadID:%hu, RefCount:%d, FPS:%d, MinFPS:%d, MaxFPS:%d\n", usIndex, nRefCount, nFPS, nMinFPS, nMaxFPS);
	}
}


void LRecvThreadManager::ReleaseRecvThreadManagerResource()
{ 
	if (m_parrRecvDesc == NULL)
	{
		return ;
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		if (m_parrRecvDesc[usIndex].pRecvThread != NULL)
		{
			m_parrRecvDesc[usIndex].pRecvThread->ReleaseRecvThreadResource();
			delete m_parrRecvDesc[usIndex].pRecvThread;
			m_parrRecvDesc[usIndex].pRecvThread = NULL;
		}
	}
	delete[] m_parrRecvDesc;
	m_parrRecvDesc = NULL;
}


