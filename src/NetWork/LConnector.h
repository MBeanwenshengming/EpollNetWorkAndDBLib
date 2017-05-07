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

#ifndef LCONNECTOR_H_
#define LCONNECTOR_H_

#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"
#include "LVarLenCircleBuf.h"
#include "LSocket.h"
#include "IncludeHeader.h"

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
	virtual ~LConnector();
public:
	//	szRemoteIP  需要连接的服务器的IP地址
	//	usPort		需要连接的服务器的端口号
	//	usReConnectTime 		  连接断开之后，重新连接的间隔时间
	//	unRecvedPacketsBufLen  接收的连接的数据包缓存长度
	//	unSendPacketBufLen	  发送队列的缓存的长度
	//	unBufUseToRecvData	  用来接收数据的临时缓存长度，还有没有接收完的部分数据的缓存
	bool Initialize(char szRemoteIP[33], unsigned short usPort, unsigned short usReConnectTime,
			unsigned int unRecvedPacketsBufLen, unsigned int unSendPacketBufLen, unsigned int unBufUseToRecvData, unsigned int unMaxPacketLen);
	void CheckMachineIsLittleEndian();
public:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();


public:		//	主逻辑线程调用
	//	获取一个接收到的数据包
	bool GetOneRecvedPacket(char* pBufToGetData, unsigned int unBufLen, unsigned int& unGettedDataLen);
	//	发送一个数据包
	bool SendOnePacket(char* pDataToSend, unsigned int unDataLen);
	//	是否处于连接状态
	bool IsConnectted();

protected:	//	重新连接
	bool ReConnect();
	int ParseRecvedData();
	int SendPackets();
	bool AddToEpollHandle(int nSocket, bool bFirstAdd);
	int DoRecv();
private:
	LVarLenCircleBuf m_RecvedPackets;
	LFixLenCircleBuf m_WillSendPackets;
	char m_szRemoteIP[33];
	unsigned short m_usRemotePort;
	unsigned short m_usReConnectTime;	//	重新连接间隔时间,单位为秒
	char* m_pszBufUseToRecvData;
	unsigned int m_unDataRemainInRecvBuf;
	unsigned int m_unBufUseToRecvDataLen;
	LSocket m_Socket;
	volatile int m_nConnected;				//	当前是否连接上了服务器
	unsigned int m_unMaxPacketLen;		//	最大的数据包长度
	time_t m_tLastConnnectTime;			//	上一次连接的时间
	bool m_bSendable;

	fd_set m_RdSet;							//	读取Set
	fd_set m_WrSet;							//	写Set

public:
	int m_nRecvedPackets;				//	 接收线程接收到的数据包数量
	int m_nRecvedPacketsMainLogic;	//	主线程处理的数据包数量
	int m_nMainLogicSendBytes;			//	主线程发送的数量
	int m_nRealSendedBytes;				//	实际发送的数据包数量
	int m_nRecvedPacketsBytes;			//	接收到的数据包的字节数
public:	//	log相关
	void SetLogTypeAndLevel(unsigned int unLogType, unsigned int unLevel);
	void GetLogTypeAndLevel(unsigned int& unLogType, unsigned int& unLevel);
	void SetInfoRecorderBase(LInfoRecorderBase* pInfoRecorderBase);
	void InfoRecorder(unsigned int unLogType, unsigned int unLevel, unsigned int unLogSubType, unsigned int unThreadID, const char* pszFileName, const char* pszFunctionName, int nLine, const char* pszFmt, ...);
private:
	LInfoRecorderBase* m_pInfoRecorderBase;
};

#endif /* LCONNECTOR_H_ */
