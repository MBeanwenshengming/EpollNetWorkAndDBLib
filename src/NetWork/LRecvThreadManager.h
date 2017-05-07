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

#ifndef __LINUX_RECV_THREAD_MANAGER_HEADER_DEFINDED__
#define __LINUX_RECV_THREAD_MANAGER_HEADER_DEFINDED__

#include "LRecvThread.h"
class LSession;
class LNetWorkServices;

typedef struct _Recv_Thread_Desc
{
	unsigned short usThreadID;
	volatile int nBindSessionCount;
	LRecvThread* pRecvThread;
	_Recv_Thread_Desc()
	{
		usThreadID = 0;
		__sync_lock_test_and_set(&nBindSessionCount, 0);
		pRecvThread = NULL;
	}
}t_Recv_Thread_Desc;

class LRecvThreadManager
{
public:
	LRecvThreadManager();
	~LRecvThreadManager();
public:
	bool StartAllRecvThread();
	bool StopAllRecvThread();
public:

	//	unEpollThreadCount  EpollThread线程数量
	//	usThreadCount 接收线程的数量
	//	unRecvWorkItemCount epollin事件环形队列的最大队列数，大于该数量，那么EPOLLIN事件无法放入接收线程处理
	//	unCloseWorkItemCount 处理关闭连接的缓存队列长度
	bool Init(unsigned int unEpollThreadCount, unsigned short usThreadCount, unsigned int unRecvWorkItemCount, unsigned int unCloseWorkItemCount);
	bool BindRecvThread(LSession* pSession); 
	bool UnBindRecvThread(LSession* pSession);

	LRecvThread* GetRecvThread(unsigned short usThreadID);
protected:
	int SelectRecvThread(); 
private:
	t_Recv_Thread_Desc* m_parrRecvDesc;
	unsigned short m_usThreadCount;


public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices);
	void ReleaseRecvThreadManagerResource();
private:
	LNetWorkServices* m_pNetWorkServices;

public:	//	for Test
	void PrintSendThreadRefStatus();	
};
#endif

