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

#include "LSendThreadManager.h"
#include "LSendThread.h"
#include "LSession.h"
#include "LNetWorkServices.h" 


LSendThreadManager::LSendThreadManager()
{
	m_parrSendThreadDesc 	= NULL;
	m_pNetWorkServices 		= NULL;
	m_unSendThreadCount		= 0;
}
LSendThreadManager::~LSendThreadManager()
{
}

bool LSendThreadManager::Initialize(unsigned int unEpollThreadCount, unsigned int unSendThreadCount, unsigned int unSendWorkItemCount, unsigned int unEpollOutEventMaxCount, unsigned int unCloseWorkItemCount)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	if (unSendThreadCount == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::Initialize", __LINE__,
															"unSendThreadCount == 0\n");
		return false;
	}
	m_unSendThreadCount = unSendThreadCount;

	m_parrSendThreadDesc = new t_Send_Thread_Desc[m_unSendThreadCount];
	if (m_parrSendThreadDesc == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::Initialize", __LINE__,
																	"m_parrSendThreadDesc == NULL, SendThreadCount:%u\n", m_unSendThreadCount);
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		m_parrSendThreadDesc[unIndex].pSendThread = new LSendThread;
		if (m_parrSendThreadDesc[unIndex].pSendThread == NULL)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::Initialize", __LINE__,
																				"m_parrSendThreadDesc[%u].pSendThread == NULL\n", unIndex);
			return false;
		}
		m_parrSendThreadDesc[unIndex].pSendThread->m_nThreadID = unIndex + 1;
		m_parrSendThreadDesc[unIndex].pSendThread->SetNetWorkServices(m_pNetWorkServices); 
		if (!m_parrSendThreadDesc[unIndex].pSendThread->Initialize(unSendWorkItemCount, unEpollThreadCount, unEpollOutEventMaxCount, unCloseWorkItemCount))
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::Initialize", __LINE__,
														"m_parrSendThreadDesc[%u].pSendThread->Initialize Failed\n", unIndex);
			return false;
		} 
	}
	return true;
}
int LSendThreadManager::GetCurrentPacketFreePoolItemCount()
{
	int nCount = 0;

	return nCount;
}

int LSendThreadManager::SelectSendThread()
{
	int nSelectedThreadID = -1;
	int nMinSessionCount = 0x0fffffff;
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		int nCurrentRefCount = __sync_add_and_fetch(&m_parrSendThreadDesc[unIndex].nRefCount, 0);
		if (nCurrentRefCount < nMinSessionCount)
		{
			nMinSessionCount = nCurrentRefCount;
			nSelectedThreadID = unIndex;
		}
	}
	return nSelectedThreadID;
} 
bool LSendThreadManager::BindSendThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::BindSendThread", __LINE__,
																"pSession == NULL\n");
		return false;
	}
	int nSelectedThreadID = SelectSendThread();
	nSelectedThreadID += 1;
	if (nSelectedThreadID <= 0 || nSelectedThreadID > m_unSendThreadCount)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::BindSendThread", __LINE__,
																		"SelectedThreadID:%d, SendThreadCount:%u\n", nSelectedThreadID, m_unSendThreadCount);
		return false;
	}
	pSession->SetSendThreadID(nSelectedThreadID);

	__sync_add_and_fetch(&m_parrSendThreadDesc[nSelectedThreadID - 1].nRefCount, 1);
	return true;
}
void LSendThreadManager::UnBindSendThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::UnBindSendThread", __LINE__,
																		"pSession == NULL\n");
		return ;
	}
	int nSendThreadID = pSession->GetSendThreadID();
	if (nSendThreadID <= 0 || nSendThreadID > m_unSendThreadCount)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::UnBindSendThread", __LINE__,
													"SessionSendThreadID:%d, SendThreadCount:%u\n", nSendThreadID, m_unSendThreadCount);
		return ;
	}
	__sync_sub_and_fetch(&(m_parrSendThreadDesc[nSendThreadID - 1].nRefCount), 1);
	pSession->SetSendThreadID(-1);
}

void LSendThreadManager::SetNetWorkServices(LNetWorkServices* pNetWorkServices) 
{
	m_pNetWorkServices = pNetWorkServices;
}

	
LSendThread* LSendThreadManager::GetSendThread(unsigned int unThreadID)
{
	if (unThreadID > m_unSendThreadCount || unThreadID == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::GetSendThread", __LINE__,
															"ThreadID:%u, SendThreadCount:%u\n", unThreadID, m_unSendThreadCount);
		return NULL;
	}
	return m_parrSendThreadDesc[unThreadID - 1].pSendThread;
}


bool LSendThreadManager::StartAllSendThread()
{
	if (m_parrSendThreadDesc == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::StartAllSendThread", __LINE__,
																	"m_parrSendThreadDesc == NULL\n");
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		if (!m_parrSendThreadDesc[unIndex].pSendThread->Start())
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::StartAllSendThread", __LINE__,
																				"m_parrSendThreadDesc[%u].pSendThread->Start Failed\n", unIndex);
			return false;
		}
	}
	return true;
}

void LSendThreadManager::StopAllSendThread()
{
	if (m_parrSendThreadDesc == NULL)
	{
		return ;
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		if (m_parrSendThreadDesc[unIndex].pSendThread != NULL)
		{ 
			m_parrSendThreadDesc[unIndex].pSendThread->Stop();
		}
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		if (m_parrSendThreadDesc[unIndex].pSendThread != NULL)
		{
			pthread_t pID = m_parrSendThreadDesc[unIndex].pSendThread->GetThreadHandle();
			if (pID == 0)
			{
				continue;
			}
			int nJoinRes = pthread_join(pID, NULL);
			if (nJoinRes != 0)
			{
				m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LSendThreadManager::StopAllSendThread", __LINE__,
																								"pthread_join Failed, ThreadIndex:%u, SystemErrorID:%d\n", unIndex, errno);
			}
		}
	}
}


void LSendThreadManager::PrintSendThreadRefStatus()
{
	printf("\tSendThreadManager, Send Thread RefCount\n");
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		int nRefCount = m_parrSendThreadDesc[unIndex].nRefCount;
		int nFPS = m_parrSendThreadDesc[unIndex].pSendThread->GetFPS();
		int nMinFPS = m_parrSendThreadDesc[unIndex].pSendThread->GetMinFPS();
		int nMaxFPS = m_parrSendThreadDesc[unIndex].pSendThread->GetMaxFPS();
		printf("\t\tSendThreadID:%d, RefCount:%d, FPS:%d, MinFPS:%d, MaxFPS:%d\n", unIndex, nRefCount, nFPS, nMinFPS, nMaxFPS);
	}
}

void LSendThreadManager::ReleaseSendThreadManagerResource()
{
	if (m_parrSendThreadDesc == NULL)
	{
		return ;
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		if (m_parrSendThreadDesc[unIndex].pSendThread != NULL)
		{
			m_parrSendThreadDesc[unIndex].pSendThread->ReleaseSendThreadResource();
			delete m_parrSendThreadDesc[unIndex].pSendThread;
			m_parrSendThreadDesc[unIndex].pSendThread = NULL;
		}
	}
	delete[] m_parrSendThreadDesc;
	m_parrSendThreadDesc = NULL;
}
