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

#include "LEpollThreadManager.h"
#include "LRecvThread.h"
#include "LEpollThread.h"
#include "LSession.h"
#include "LNetWorkServices.h"


extern bool g_bEpollETEnabled;


LEpollThreadManager::LEpollThreadManager()
{
	m_parrEpollThreadManager 	= NULL;	
	m_pNetWorkServices			= NULL;
	m_unEpollThreadCount		= 0;
}
LEpollThreadManager::~LEpollThreadManager()
{
}	

bool LEpollThreadManager::Initialize(unsigned int unEpollThreadCount, unsigned int unWaitClientSizePerEpoll, unsigned int unCloseWorkItemCount)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	if (unEpollThreadCount == 0)
	{
		unEpollThreadCount = 1; 
	}
	m_unEpollThreadCount = unEpollThreadCount;
	m_parrEpollThreadManager = new t_Epoll_Thread_Desc[unEpollThreadCount];
	if (m_parrEpollThreadManager == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::Initialize", __LINE__,
											"m_parrEpollThreadManager == NULL, EpollThreadCount:%u\n", unEpollThreadCount);
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < unEpollThreadCount; ++unIndex)
	{
		m_parrEpollThreadManager[unIndex].pEpollThread = new LEpollThread;
		if (m_parrEpollThreadManager[unIndex].pEpollThread == NULL)
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::Initialize", __LINE__,
														"m_parrEpollThreadManager[%u].pEpollThread == NULL\n", unIndex);
			return false;
		}
		m_parrEpollThreadManager[unIndex].pEpollThread->SetNetWorkServices(m_pNetWorkServices); 
		m_parrEpollThreadManager[unIndex].pEpollThread->m_nThreadID = unIndex + 1;			//	线程ID从1开始
		if (!m_parrEpollThreadManager[unIndex].pEpollThread->Initialize(unWaitClientSizePerEpoll, unCloseWorkItemCount))
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::Initialize", __LINE__,
																	"m_parrEpollThreadManager[%u].pEpollThread->Initialize Failed, WaitClientSizePerEpoll:%u, CloseWorkItemCount:%u\n",
																	unIndex, unWaitClientSizePerEpoll, unCloseWorkItemCount);
			return false;
		}
	}

	return true;
}
bool LEpollThreadManager::StartAllEpollThread()
{
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		if (!m_parrEpollThreadManager[unIndex].pEpollThread->Start())
		{
			m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::StartAllEpollThread", __LINE__,
																				"m_parrEpollThreadManager[%u].pEpollThread->Start() Failed\n", unIndex);
			return false;
		} 
	}
	return true;
}
void LEpollThreadManager::StopAllEpollThread()
{
	if (m_parrEpollThreadManager != NULL)
	{
		for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
		{
			//	关闭线程
			if (m_parrEpollThreadManager[unIndex].pEpollThread != NULL)
			{
				m_parrEpollThreadManager[unIndex].pEpollThread->Stop();
			}
		} 
		for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
		{
			if (m_parrEpollThreadManager[unIndex].pEpollThread != NULL)
			{
				//	等待线程关闭
				pthread_t pID = m_parrEpollThreadManager[unIndex].pEpollThread->GetThreadHandle(); 
				if (pID != 0)
				{
					int nJoinResult = pthread_join(pID, NULL);
					if (nJoinResult != 0)
					{
						m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::StopAllEpollThread", __LINE__,
																										"pthread_join Failed, errorID:%d, ThreadIndex:%u\n", errno, unIndex);
					}
				}
			}
		}
	}
}

bool LEpollThreadManager::PostEpollReadEvent(LSession* pSession)
{
	if (pSession == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::PostEpollReadEvent", __LINE__,
																												"pSession == NULL\n");
		return false;
	}
	int nThreadID = pSession->GetEpollThreadID();
	struct epoll_event epollEvent;
	memset(&epollEvent, 0, sizeof(epollEvent));

	t_Epoll_Bind_Param* pEpollBindParam = pSession->GetEpollBindParam();
	if (g_bEpollETEnabled)
	{ 
		epollEvent.events = EPOLLIN | EPOLLET;
		//epollEvent.data.ptr = pEpollBindParam;
		epollEvent.data.u64 = pEpollBindParam->u64SessionID;
	}
	else
	{
		epollEvent.events = EPOLLIN | EPOLLONESHOT;
		//epollEvent.data.ptr = pEpollBindParam;
		epollEvent.data.u64 = pEpollBindParam->u64SessionID;
	}
	int nBindEpollSuccess = epoll_ctl(m_parrEpollThreadManager[nThreadID].pEpollThread->GetEpollHandle(), EPOLL_CTL_ADD, pEpollBindParam->nSocket, &epollEvent);  
	if (nBindEpollSuccess == -1)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::PostEpollReadEvent", __LINE__,
																														"epoll_ctl Failed, Session:%llu\n", pSession->GetSessionID());
		return false;
	}
	return true;
}

bool LEpollThreadManager::BindEpollThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::BindEpollThread", __LINE__,
																														"pSession == NULL\n");
		return false;
	}
	int nThreadID = SelectEpollThread();
	nThreadID += 1;
	if (nThreadID <= 0 || nThreadID > m_unEpollThreadCount)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::BindEpollThread", __LINE__,
																																"nThreadID <= 0 || nThreadID > m_unEpollThreadCount, ThreadID:%d, EpollThreadCount:%u\n", nThreadID, m_unEpollThreadCount);
		return false;
	}

	uint64_t uSessionID = pSession->GetSessionID();
	int nSessionSocket = pSession->GetSocket(); 
	pSession->SetEpollBindParam(uSessionID, nSessionSocket);

	pSession->SetEpollThreadID(nThreadID); 
	__sync_add_and_fetch(&m_parrEpollThreadManager[nThreadID - 1].nRefCount, 1);
	return true;
}
void LEpollThreadManager::UnBindEpollThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::UnBindEpollThread", __LINE__,
																																"pSession == NULL\n");
		return ;
	}

	int nEpollThreadID = pSession->GetEpollThreadID();
	if (nEpollThreadID <= 0 || nEpollThreadID > m_unEpollThreadCount)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::UnBindEpollThread", __LINE__,
				"nEpollThreadID <= 0 || nEpollThreadID > m_unEpollThreadCount, ThreadID:%d, EpollThreadCount:%u, SessionID:%llu\n",
				nEpollThreadID, m_unEpollThreadCount, pSession->GetSessionID());
		return ;
	}
	__sync_sub_and_fetch(&m_parrEpollThreadManager[nEpollThreadID - 1].nRefCount, 1);

	pSession->SetEpollThreadID(-1);
}

int LEpollThreadManager::SelectEpollThread()
{
	int nMinEpollRefCount = 0x0fffffff;
	int nSelectted = -1;
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		int nRefCount = m_parrEpollThreadManager[unIndex].nRefCount;
		if (nRefCount < nMinEpollRefCount)
		{
			nMinEpollRefCount = nRefCount;
			nSelectted = unIndex;
		}
	}
	return nSelectted;
}
void LEpollThreadManager::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
{ 
	m_pNetWorkServices	= pNetWorkServices;
}

LEpollThread* LEpollThreadManager::GetEpollThread(unsigned int unThreadID)
{
	if (unThreadID > m_unEpollThreadCount || unThreadID == 0)
	{
		m_pNetWorkServices->InfoRecorder(LOG_TYPE_MANAGER, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 0, __FILE__, "LEpollThreadManager::GetEpollThread", __LINE__,
																																		"unThreadID > m_unEpollThreadCount || unThreadID == 0, ThreadID:%u, EpollThreadCount:%u\n", unThreadID, m_unEpollThreadCount);
		return NULL;
	}
	return m_parrEpollThreadManager[unThreadID - 1].pEpollThread;
}


void LEpollThreadManager::PrintEpollThreadStatus()
{
	char szInfo[] = "\tLEpollThreadManager Epoll Thread RefCount";
	printf("%s\n", szInfo);
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		char szInfoForEach[512];
		int nRefCount = m_parrEpollThreadManager[unIndex].nRefCount;
		int nFPS = m_parrEpollThreadManager[unIndex].pEpollThread->GetFPS();
		int nMinFPS = m_parrEpollThreadManager[unIndex].pEpollThread->GetMinFPS();
		int nMaxFPS = m_parrEpollThreadManager[unIndex].pEpollThread->GetMaxFPS();
		sprintf(szInfoForEach, "EpollThreadID:%d, RefCount:%d, FPS:%d, MinFPS:%d, MaxFPS:%d", unIndex, nRefCount, nFPS, nMinFPS, nMaxFPS);
		printf("\t\t%s\n", szInfoForEach);
	}
}

//	释放EpollThread使用的所有资源
void LEpollThreadManager::ReleaseEpollThreadManagerResource()
{ 
	if (m_parrEpollThreadManager == NULL)
	{
		return ;
	}
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		if (m_parrEpollThreadManager[unIndex].pEpollThread != NULL)
		{
			m_parrEpollThreadManager[unIndex].pEpollThread->ReleaseEpollThreadResource();
			delete m_parrEpollThreadManager[unIndex].pEpollThread;
			m_parrEpollThreadManager[unIndex].pEpollThread = NULL;
		}
	}
	delete[] m_parrEpollThreadManager;
}

