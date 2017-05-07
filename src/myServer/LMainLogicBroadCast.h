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

#ifndef __LINUX_MAIN_LOGIC_BROADCAST_HEADER_INCLUDED__
#define  __LINUX_MAIN_LOGIC_BROADCAST_HEADER_INCLUDED__

class LNetWorkServices;
#include "LThreadBase.h"
#include <map>
using namespace std;
#include "LServerBaseNetWork.h"

typedef struct _Session_Info
{
	uint64_t u64SessionID;
	unsigned int unSendThreadID;
	int nLastRecvPacketID;
	_Session_Info()
	{
		u64SessionID 	= 0;
		unSendThreadID = 0;
		nLastRecvPacketID = 0;
	}
}t_Session_Info;

class LPacketSingle;

class LMainLogicBroadCast : public LServerBaseNetWork, public LThreadBase
{
public:
	LMainLogicBroadCast();
	~LMainLogicBroadCast();

public:		//	线程模型, 虚函数
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();

public:			//	对公网络虚函数
	virtual bool OnAddSession(uint64_t u64SessionID, char* pszRemoteIP, unsigned short usRemotePort, int nRecvThreadID, int nSendThreadID);
	virtual void OnRemoveSession(uint64_t u64SessionID);
	virtual void OnRecvedPacket(uint64_t u64SessionID, const char* pPacketData, unsigned int unPacketDataLen);

public:
	bool Initialize(char* pszConfigFile);
	bool BuildRandomPacket(LPacketSingle* pPacket, bool bBroadCast, int& nPacketSendIndex);
private:
	map<uint64_t, t_Session_Info> m_mapSessionManaged;


private:
	time_t m_tLastWriteTime;
	int 	m_nSendPacketFreeCount;

	int m_nLastBroadCastID;
	uint64_t m_u64PacketProcessed;
};
#endif

