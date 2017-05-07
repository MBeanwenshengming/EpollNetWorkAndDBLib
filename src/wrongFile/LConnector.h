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

#ifndef __LINUX_SINGLE_CONNECTOR_HEADER_DEFINED__
#define __LINUX_SINGLE_CONNECTOR_HEADER_DEFINED__

#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"
#include "LSocket.h"
#include "LVarLenCircleBuf.h"

class LInfoRecorderBase;

typedef struct _Connector_Initialize_Params
{
	char szIP[128];					//	连接IP
	unsigned short usPort;			//	连接端口号
	unsigned int unReConnectPeriodTime;		//	断开后的重连间隔
	unsigned int unMaxPacketLen;	//	单个最大协议包长度
	unsigned int unMaxRecvBufLen;	//	用来接收数据的缓存长度
	unsigned int unMaxSendBufLen;	//	发送缓存的长度
	unsigned int unRecvedPacketBufLen;	//	接收到的数据包的buf长度
	_Connector_Initialize_Params()
	{
		memset(szIP, 0, sizeof(szIP));	
		usPort 							= 0;
		unReConnectPeriodTime 		= 10;
		unMaxPacketLen					= 0;
		unMaxRecvBufLen				= 0;
		unMaxSendBufLen				= 0;
		unRecvedPacketBufLen			= 0;
	} 
}t_Connector_Initialize_Params;

class LConnectorConfigProcessor
{
public:
	LConnectorConfigProcessor();
	~LConnectorConfigProcessor();
public:
	bool Initialize(char* pConfigFileName, char* pSectionHeader, bool bReadIpAndPort = true);
	t_Connector_Initialize_Params& GetConnectorInitializeParams();
private:
	t_Connector_Initialize_Params m_Cip;
};

class LConnector : public LThreadBase
{
public:
	LConnector();
	~LConnector();
private:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();
public:		// 

private:
	int CheckForSocketEvent();
	int OnRecv();
	int OnSend();
	bool StopThreadAndStopConnector();
public:
	LSocket* GetSocket(); 

	//	nReconnectPeriodTime  seconds
	bool Initialize(t_Connector_Initialize_Params& cip, bool bConnectImmediate = false);
protected:
	bool ReConnect();
private:
	LSocket m_Socket;
	int m_nReconnectPeriodTime;
	char m_szIP[128];
	unsigned short m_usPort;
	time_t m_nLastestConnected;

private:
	int ParseRecvedData(char* pData, unsigned int unDataLen);
private:
	char* m_pszBufForRecv;
	unsigned int m_unBufForRecvLen;		//	接收buf的长度
	unsigned int m_unRemainedDataLen;	//	剩余的还没有解析的数据包
	unsigned int m_unMaxPacketLen;		//	最大单个数据包长度

public:
	//	获取一个接受到的数据包
	int GetOneRecvedPacket(char* pDataBuf, unsigned int unDataBufLen, unsigned int& unDataGettedLen);
private:
	LVarLenCircleBuf m_RecvedPacket;		//	接收到的数据包全部放在这里


public:
	//	释放占用的资源
	void ReleaseConnectorResource();

public:
	//	发送数据长度
	int SendPacket(char* pData, unsigned int unDataLen);
protected:
	int SendData();
private:
	bool 	m_bSendable;
	LFixLenCircleBuf m_FixLenCircleBufForSend;		//	要发送的数据全部放在这里
	char m_szTempSendDataBuf[128 * 1024];
	unsigned short m_usTempSendDataLen;
private:
	fd_set m_rdset;
	fd_set m_wrset;

public: 
	bool IsConnectted()
	{
		int nIsConnectted = m_nIsConnect;
		if (nIsConnectted == 1)
		{
			return true;
		}
		return false;
	}
private:
	void SetIsConnnectted(int i)
	{
		__sync_lock_test_and_set(&m_nIsConnect, i);
	}
	volatile int m_nIsConnect;

public:
	void SetLogTypeAndLevel(unsigned int unLogType, unsigned int unLevel);
	void GetLogTypeAndLevel(unsigned int& unLogType, unsigned int& unLevel);
	void SetInfoRecorderBase(LInfoRecorderBase* pInfoRecorderBase);
	void InfoRecorder(unsigned int unLogType, unsigned int unLevel, unsigned int unLogSubType, unsigned int unThreadID, const char* pszFileName, const char* pszFunctionName, int nLine, const char* pszFmt, ...);
private:
	LInfoRecorderBase* m_pInfoRecorderBase;

};
#endif



