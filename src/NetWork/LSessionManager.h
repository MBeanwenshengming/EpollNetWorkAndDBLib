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

#ifndef __LINUX_SESSION_MANAGER_HEADER_DEFINED__
#define __LINUX_SESSION_MANAGER_HEADER_DEFINED__

#include "IncludeHeader.h"
#include "LSession.h"
#include <map>
#include <queue>
#include <list>

using namespace std;
class LNetWorkServices;
typedef struct _Session_Manager_Slot_Info
{
	int nSlotIndex;		//	数组下标
	volatile int nSessionIndex;	//	该下标下的SesssionIndex，最终的SessionID = nSlotIndex << 32 | nSessionIndex,组成64位的SessionID
	LSession* pSession;
	_Session_Manager_Slot_Info()
	{
		nSlotIndex = 0;
		__sync_lock_test_and_set(&nSessionIndex, 0);
		pSession = NULL;
	}
}t_Session_Manager_Slot_Info;


class LMasterSessionManager
{
public:
	LMasterSessionManager();
	~LMasterSessionManager();

public:
	void FreeSessionToPool(LSession* pSession);

private: 
	queue<LSession*> m_queueDirtySessionManager;		//	Session pool
	pthread_mutex_t m_mutexForDirty;						//	Session pool 保护

public:
	bool InitializeSessionPool(unsigned int unMaxClientNum, unsigned int unRecvBufSizeLen, unsigned int unSendBufSizeLen);
	LSession* AllocSession(int nSessionSocket);
	LSession* FindSession(uint64_t u64SessionID);
	void FreeSession(LSession* pSession);
	void Release();
	bool GetSessionIPAndPort(uint64_t u64SessionID, char* pszBuf, unsigned short usbufLen);
	void PrintAllSessionInfos();
private:
	//	当前在线的连接数
	int m_nCurrentOnlineSessionNum;
	queue<int> m_SessionManagerSlotFree;								//	当前空闲的Slot，被m_mutexForDirty保护
	t_Session_Manager_Slot_Info* m_pArraySessionManagerSlotInfo;
	unsigned int m_unSlotNum;

public:
	//	删除固定时间没有通讯的连接
	void KickOutIdleSession();
private:
	list<uint64_t> m_listCurrentOnlineClient;			//	当前在线的连接
	//	新的连接到来，需要把sessionID放到这里，然后在KickOutIdleSession里面将SessionID放到m_listCurrentOnlineClient里面处理
	//	AllocSession在AcceptThread调用，执行写操作，KickOutIdleSession在主线程调用，执行读操作，省掉一个锁
	LFixLenCircleBuf m_cFixLenCircleBufNewSessionOnline;
public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices)
	{
		m_pNetWorkServices = pNetWorkServices;
	}
private:
	LNetWorkServices* m_pNetWorkServices;
public:
	void SetTimeForKickOutIdleSession(unsigned short usTime)
	{
		if (usTime != 0)
		{
			m_usTime = usTime;
		}
	}
	unsigned short GetTimeForKickOutIdleSession()
	{
		return m_usTime;
	}
private:
	unsigned short m_usTime; 	// 踢出未通信连接的时间
public:
	//	释放所有的Session和SessionManager，以及它们占用的资源
	void ReleaseMasterSessionManagerResource();

public:
	void AddToWillReuseManager(LSession* pSession);
	size_t GetWillReuseSessionCount();
	void MoveWillCloseSessionToSessionPool(LSession* pSession);
private:
	//	准备释放重用的连接管理器，Session从Manager里面移除之后，放在这个管理器中
	//	等Epoll线程recv线程send线程和主线程(如果会操作Session的话)告知全部不使用了，那么这里的Session才放入SessionPool里面
	pthread_mutex_t m_MutexProtectedForWillReuseManager;		//	manager的保护器，操作这个的是多线程
	map<LSession*, LSession*> m_mapSessionWillReuseManager;
};

#endif

