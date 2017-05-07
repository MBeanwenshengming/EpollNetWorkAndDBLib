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

#ifndef __LINUX_SERVER_BASE_NET_WORK__
#define __LINUX_SERVER_BASE_NET_WORK__

#include "IncludeHeader.h"
#include "LRecvThread.h"

class LNetWorkServices;

#define MAX_TEMP_SEND_DATA_BUF_LEN 256 * 1024

class LServerBaseNetWork
{

public:
	LServerBaseNetWork();
	virtual ~LServerBaseNetWork();
public:
	bool InitializeNetWork(char* pConfigFile, unsigned int unMaxNumProcessPacketOnce, char* pSectionHeader, bool bInitializeAccept = true);
	bool NetWorkStart();
	void NetWorkDown();
public:
	virtual bool OnAddSession(uint64_t u64SessionID, char* pszRemoteIP, unsigned short usRemotePort, int nRecvThreadID, int nSendThreadID);
	virtual void OnRemoveSession(uint64_t u64SessionID);
	virtual void OnRecvedPacket(uint64_t u64SessionID, const char* pPacketData, unsigned int unPacketDataLen);
public:
	//	主线程循环中，需要调用这个函数，保证网络引擎提取和处理网络消息
	void NetWorkServicesProcess(int nProcessNum);
	int GetProcessedCount();
public:

	bool SendOnePacket(uint64_t u64SessionID, char* pData, unsigned int unDataLen);
private:
	char m_szTempSendData[MAX_TEMP_SEND_DATA_BUF_LEN];
	unsigned short m_usTempDataLen;
public:
	void KickOutOneSession(uint64_t u64SessionID);

	LNetWorkServices* GetNetWorkServices()
	{
		return m_pNetWorkServices;
	}

	void SetLogTypeAndLevel(unsigned int unLogType, unsigned int unLevel);
	void GetLogTypeAndLevel(unsigned int& unLogType, unsigned int& unLevel);
	void SetInfoRecorderBase(LInfoRecorderBase* pInfoRecorderBase);
private: 
	LNetWorkServices* m_pNetWorkServices;

public:
	void GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort);
};
#endif

