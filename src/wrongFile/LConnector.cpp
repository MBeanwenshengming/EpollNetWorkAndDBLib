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

#include "LConnector.h"
#include "LIniFileReadAndWrite.h"


LConnector::LConnector()
{
	//	data for connect
	m_Socket.SetSocket(-1);
	m_nReconnectPeriodTime 	= 5;


	SetIsConnnectted(0);

	memset(m_szIP, 0, sizeof(m_szIP));
	m_usPort 				= 0;
	m_nLastestConnected	= 0;
	m_bSendable				= true;

	m_pszBufForRecv		= 0;
	m_unBufForRecvLen		= 0;	//	接收buf的长度
	m_unRemainedDataLen	= 0;
	m_nIsConnect			= 0;
	m_unMaxPacketLen		= 0;

	memset(m_szTempSendDataBuf, 0, 128 * 1024);
	m_usTempSendDataLen	= 0;
}

LConnector::~LConnector()
{ 
}

int LConnector::ThreadDoing(void* pParam)
{
	while (1)
	{
		//	如何没有连接或者连接断开，那么重连
		if (m_nIsConnect == 0)
		{
			if (!ReConnect())
			{
				 //sched_yield();
				struct timespec timeReq;
				timeReq.tv_sec 	= 0;
				timeReq.tv_nsec = 10;
				nanosleep(&timeReq, NULL);
				InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ThreadDoing", __LINE__,
																				"ReConnect Failed\n");
			}
			//	防止连接不上，但是线程一直执行无法停止
			if (CheckForStop())
			{
				InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ThreadDoing", __LINE__,
									"CheckForStop()\n");
				return 0;
			}
			continue;
		}
		// 检查网络事件
		int nWorkCount = CheckForSocketEvent();
		if (nWorkCount < 0)
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ThreadDoing", __LINE__,
					"CheckForSocketEvent, nWorkCount < 0, Thread Break\n");
			break;
		}
		// 发送在本地队列的数据		
		int nSendWorkCount = SendData();
		if (nWorkCount == 0 && nSendWorkCount == 0)
		{
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec 	= 10;
			nanosleep(&timeReq, NULL);
		}
		if (CheckForStop())
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ThreadDoing", __LINE__,
												"CheckForStop()  2\n");
			return 0;
		}
	}

	return 0; 
}
bool LConnector::OnStart()
{
	return true;
}
void LConnector::OnStop()
{
}

int LConnector::CheckForSocketEvent()
{
	if (m_nIsConnect == 0)
	{
		return 1;
	}

	FD_ZERO(&m_rdset);
	FD_ZERO(&m_wrset);
	FD_SET(m_Socket.GetSocket(), &m_rdset);
	FD_SET(m_Socket.GetSocket(), &m_wrset);

	struct timeval val;
	val.tv_sec 		= 0;
	val.tv_usec 	= 0;

	int iReturn 	= 0;
	if (!m_bSendable)
	{ 
		iReturn = select(m_Socket.GetSocket() + 1, &m_rdset, &m_wrset, NULL, &val);	
	}
	else
	{
		iReturn = select(m_Socket.GetSocket() + 1, &m_rdset, NULL, NULL, &val);	
	}
	if (iReturn == -1)
	{
		if (errno == EINTR)
		{
			return 1;
		}
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::CheckForSocketEvent", __LINE__,
																"select Failed, SystemErrorCode:%d\n", errno);
		return -1;
	}
	else if (iReturn == 0)
	{
		return 0;
	}
	if (FD_ISSET(m_Socket.GetSocket(), &m_rdset))		//	可以接收数据
	{
		OnRecv();
	}
	if (FD_ISSET(m_Socket.GetSocket(), &m_wrset))		//	可以继续发送数据了
	{
		OnSend();
	}
	return 1; 
}
int LConnector::OnRecv()
{
	ssize_t sRecved = recv(m_Socket.GetSocket(), m_pszBufForRecv + m_unRemainedDataLen, m_unBufForRecvLen - m_unRemainedDataLen, 0);
	if (sRecved == 0)	//	连接断开
	{
		SetIsConnnectted(0);
		close(m_Socket.GetSocket());
		m_Socket.SetSocket(-1);
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::OnRecv", __LINE__,
																						"recv Failed, SystemErrorCode:%d\n", errno);
		return -1;
	}
	else if (sRecved > 0)	//	接收到数据
	{
		m_unRemainedDataLen += sRecved;
		int  nParsedDataLen = ParseRecvedData(m_pszBufForRecv, m_unRemainedDataLen);
		if (nParsedDataLen < 0)	//	协议出错，需要断开连接
		{
			SetIsConnnectted(0);
			close(m_Socket.GetSocket());
			m_Socket.SetSocket(-1);
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::OnRecv", __LINE__,
																									"ParseRecvedData Failed, Return ErrorCode:%d\n", nParsedDataLen);
			return -1;
		}
		m_unRemainedDataLen -= nParsedDataLen;
		memcpy(m_pszBufForRecv, m_pszBufForRecv + nParsedDataLen, m_unRemainedDataLen);
	}
	else		//	出现错误
	{
		if (errno == EAGAIN)
		{
			return 0;
		}
		else		//	未知错误，重新连接
		{
			SetIsConnnectted(0);
			close(m_Socket.GetSocket());
			m_Socket.SetSocket(-1);
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::OnRecv", __LINE__,
																												"recv Failed, SystemErrorCode:%d\n", errno);
			return -1;
		}
	}
	return 0;
}
int LConnector::OnSend()
{
	if (!m_bSendable)
	{
		m_bSendable = true;
	}
	return 0;
}



LSocket* LConnector::GetSocket()
{
	return &m_Socket;
}

//	nReconnectPeriodTime  seconds
bool LConnector::Initialize(t_Connector_Initialize_Params& cip, bool bConnectImmediate)
{
	if (strlen(cip.szIP) == 0 || cip.usPort == 0)
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																														"strlen(cip.szIP) == 0 || cip.usPort == 0\n");
		return false;
	}
	if (cip.unMaxPacketLen == 0 || cip.unMaxRecvBufLen == 0 || cip.unMaxSendBufLen == 0 || cip.unRecvedPacketBufLen == 0)
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																																"MaxPacketLen:%u, MaxRecvBufLen:%u, MaxSendBufLen:%u,RecvedPacketBufLen:%u\n",
																																cip.unMaxPacketLen, cip.unMaxRecvBufLen, cip.unMaxSendBufLen, cip.unRecvedPacketBufLen);
		return false;
	}
	if (cip.unReConnectPeriodTime == 0)
	{
		cip.unReConnectPeriodTime = 5;
	}
	m_nReconnectPeriodTime = cip.unReConnectPeriodTime;

	strncpy(m_szIP, cip.szIP, 128);
	m_usPort = cip.usPort;
	
	//	初始化接收缓存
	m_pszBufForRecv= new char[cip.unMaxRecvBufLen];
	if (m_pszBufForRecv == NULL)
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																																"m_pszBufForRecv == NULL, MaxRecvBufLen:%u\n", cip.unMaxRecvBufLen);
		return false;
	}
	m_unBufForRecvLen = cip.unMaxRecvBufLen;

	m_unMaxPacketLen = cip.unMaxPacketLen;

	//	初始化发送缓存
	if (!m_FixLenCircleBufForSend.Initialize(sizeof(char), cip.unMaxSendBufLen))
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																						"m_FixLenCircleBufForSend.Initialize Failed, MaxSendBufLen:%u\n", cip.unMaxSendBufLen);
		return false;
	}

	//	初始化接收到的消息包的长度
	if (!m_RecvedPacket.InitializeVarLenCircleBuf(cip.unRecvedPacketBufLen))
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																								"m_RecvedPacket.InitializeVarLenCircleBuf Failed, RecvedPacketBufLen:%u\n", cip.unRecvedPacketBufLen);
		return false;
	}

	//	连接服务器
	int nSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (nSocket == -1)
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																										"nSocket == -1\n");
		return false;
	}
	sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port	= htons(m_usPort);
	//	sockaddr.sin_addr.s_addr
	int nsucc = inet_aton(m_szIP, &sockaddr.sin_addr);
	if (nsucc == 0)
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																												"inet_aton Failed, ErrorCode:%d\n", nsucc);
		return false;
	}
	if (bConnectImmediate)
	{
		nsucc = connect(nSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)); 
		if (nsucc != 0)
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																															"connect Failed, SystemErrorCode:%d\n", errno);
			return false;
		}

		SetIsConnnectted(1); 

		nsucc = fcntl(nSocket, F_SETFL, O_NONBLOCK);
		if (nsucc == -1)
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																																		"fcntl Failed, SystemErrorCode:%d\n", errno);
			return false;
		}
		m_Socket.SetSocket(nSocket);

		FD_ZERO(&m_rdset);
		FD_ZERO(&m_wrset);
		FD_SET(m_Socket.GetSocket(), &m_rdset);
		FD_SET(m_Socket.GetSocket(), &m_wrset);
	}

	if (!Start())
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::Initialize", __LINE__,
																																				"Start Failed\n");
		return false;
	}
	return true;	

}

bool LConnector::ReConnect()
{
	time_t timeNow = time(NULL);

	if ((m_nLastestConnected != 0 && timeNow - m_nLastestConnected> m_nReconnectPeriodTime) || m_nLastestConnected == 0)
	{
		m_nLastestConnected = timeNow;
		//	如果原来的socket没有关闭，那么先关闭
		if (m_Socket.GetSocket() != -1)
		{
			close(m_Socket.GetSocket());
		}

		int nSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (nSocket == -1)
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ReConnect", __LINE__,
																													"nSocket == -1\n");
			return false;
		}
		sockaddr_in sockaddr;
		memset(&sockaddr, 0, sizeof(sockaddr));
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_port	= htons(m_usPort);
		//	sockaddr.sin_addr.s_addr
		int nsucc = inet_aton(m_szIP, &sockaddr.sin_addr);
		if (nsucc == 0)
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ReConnect", __LINE__,
																															"inet_aton Failed, ErrorCode:%d\n", nsucc);
			close(nSocket);
			return false;
		}
		nsucc = connect(nSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)); 
		if (nsucc != 0)
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ReConnect", __LINE__,
																																		"connect Failed, SystemErrorCode:%d\n", errno);
			return false;
		}
		nsucc = fcntl(nSocket, F_SETFL, O_NONBLOCK);
		if (nsucc == -1)
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ReConnect", __LINE__,
																																					"fcntl Failed, SystemErrorCode:%d\n", errno);
			return false;
		}

		SetIsConnnectted(1);

		m_Socket.SetSocket(nSocket);

		FD_ZERO(&m_rdset);
		FD_ZERO(&m_wrset);
		FD_SET(m_Socket.GetSocket(), &m_rdset);
		FD_SET(m_Socket.GetSocket(), &m_wrset);
	}
	else
	{
		return false;
	}
	return true; 
}




//	停止连接线程，并且等待线程结束
bool LConnector::StopThreadAndStopConnector()
{
	Stop();

	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		int nJoinRes = pthread_join(pID, NULL);
		if (nJoinRes != 0)
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::StopThreadAndStopConnector", __LINE__,
																																								"pthread_join Failed, SystemErrorCode:%d\n", errno);
			return false;
		}
	}
	return true;
}

//	释放占用的资源
void LConnector::ReleaseConnectorResource()
{
	if (m_nIsConnect == 1)
	{
		close(m_Socket.GetSocket());
	}
	if (m_pszBufForRecv != NULL)
	{
		delete[] m_pszBufForRecv;
		m_pszBufForRecv = NULL;
	}
}


int LConnector::ParseRecvedData(char* pData, unsigned int unDataLen)
{
	//	如果现存的数据长度没有unsigned short长，那么说明数据包不完整,不需要处理
	if (unDataLen <= sizeof(unsigned short))
	{
		return 0;
	}

	int nParsedDataLen = 0;

	unsigned short usDataLen = 0;
	usDataLen = *((unsigned short*)pData);

	while (unDataLen >= usDataLen)
	{
		if (usDataLen > m_unMaxPacketLen || usDataLen <= sizeof(unsigned short))	//	连接上的数据有错误
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ParseRecvedData", __LINE__,
												"usDataLen > m_unMaxPacketLen || usDataLen <= sizeof(unsigned short), MaxPacketLen:%u, GettedLen:%hu\n", m_unMaxPacketLen, usDataLen);
			return -1;
		}

		E_Circle_Error eError = m_RecvedPacket.AddData(pData + sizeof(unsigned short), usDataLen - sizeof(unsigned short));
		if (eError != E_Circle_Buf_No_Error)	//	数据包错误，关闭连接
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_FATAL, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::ParseRecvedData", __LINE__,
															"m_RecvedPacket.AddData Failed, ErrorCode:%d\n", eError);
			return -1;
		}

		pData 		+= usDataLen;
		unDataLen 	-= usDataLen;
		nParsedDataLen += usDataLen;

		if (unDataLen >= sizeof(unsigned short))
		{
			usDataLen = *((unsigned short*)pData); 
		}
		else
		{
			break;
		}
	}
	return nParsedDataLen;
}
//	获取一个接受到的数据包
int LConnector::GetOneRecvedPacket(char* pDataBuf, unsigned int unDataBufLen, unsigned int& unDataGettedLen)
{
	E_Circle_Error eErrorCode = m_RecvedPacket.GetData(pDataBuf, unDataBufLen, unDataGettedLen);
	if (eErrorCode == E_Circle_Buf_No_Error)
	{
		return 0;
	}
	else
	{
		if (eErrorCode != E_Circle_Buf_Is_Empty)
		{
			InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::GetOneRecvedPacket", __LINE__,
																"m_RecvedPacket.GetData Failed, ErrorCode:%d\n", eErrorCode);
		}
		return eErrorCode;
	}
}
//	发送数据长度
int LConnector::SendPacket(char* pData, unsigned int unDataLen)
{
	m_usTempSendDataLen = 0;
	if (unDataLen > 128 * 1024 - 2)
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::SendPacket", __LINE__,
																		"unDataLen > 128 * 1024 - 2, DataLen:%u\n", unDataLen);
		return -10;
	}
	m_usTempSendDataLen += unDataLen + 2;
	memcpy(m_szTempSendDataBuf, &m_usTempSendDataLen, sizeof(unsigned short));
	memcpy(m_szTempSendDataBuf + sizeof(unsigned short), pData, unDataLen);
	E_Circle_Error eErrorCode = m_FixLenCircleBufForSend.AddItems(m_szTempSendDataBuf, m_usTempSendDataLen);
	if (eErrorCode == E_Circle_Buf_No_Error)
	{
		return 0;
	}
	else
	{
		InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::SendPacket", __LINE__,
																				"m_FixLenCircleBufForSend.AddItems Failed, ErrorCode:%d\n", eErrorCode);
	}
	//	设置为断开连接
	SetIsConnnectted(0);
	return eErrorCode;
}
int LConnector::SendData()
{ 
	if (m_nIsConnect == 0)
	{
		return 0;
	}
	if (!m_bSendable)
	{
		return 0;
	}

	int nContentLen = m_FixLenCircleBufForSend.GetCurrentExistCount();
	if (nContentLen == 0)
	{
		return 0;
	}
	int nReadIndex = 0;
	int nWriteIndex = 0;
	m_FixLenCircleBufForSend.GetCurrentReadAndWriteIndex(nReadIndex, nWriteIndex);
	if (nReadIndex == nWriteIndex)
	{
		return 0;
	}
	char* pBufStart = m_FixLenCircleBufForSend.GetBuf();

	if (nReadIndex < nWriteIndex)
	{
		int nResult = send(m_Socket.GetSocket(), pBufStart + nReadIndex, nWriteIndex - nReadIndex, 0);

		if (nResult == -1)
		{
			if (errno == EAGAIN)
			{
				m_bSendable = false;
				return 1;
			}
			else		//	出现其它错误，显示为连接断开，重新连接
			{
				SetIsConnnectted(0);

				close(m_Socket.GetSocket());
				m_Socket.SetSocket(-1);
				InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::SendData", __LINE__,
																								"send Failed, SystemErrorCode:%d\n", errno);
				return -1;
			}
		}
		else
		{
			m_FixLenCircleBufForSend.SetReadIndex(nWriteIndex);
			return nResult;
		}
	}
	else
	{
		//	先发送到缓冲结尾的地方，然后发送从头开始的地方
		//	第一步，发送到结尾的数据
		int nSendCount = 0;

		int nResult = send(m_Socket.GetSocket(), pBufStart + nReadIndex, m_FixLenCircleBufForSend.GetMaxItemCount() - nReadIndex, 0);
		if (nResult == -1)
		{
			if (errno == EAGAIN)
			{
				m_bSendable = false;
				return 1;
			}
			else		//	出现其它错误，显示为连接断开，重新连接
			{
				SetIsConnnectted(0);

				close(m_Socket.GetSocket());
				m_Socket.SetSocket(-1);
				InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::SendData", __LINE__,
																												"2 send Failed, SystemErrorCode:%d\n", errno);
				return -1;
			}
		}
		else
		{
			m_FixLenCircleBufForSend.SetReadIndex(0);
			nSendCount += nResult;
		}

		if (nWriteIndex == 0)	//	说明没有数据
		{
			m_FixLenCircleBufForSend.SetReadIndex(nWriteIndex);
			return nSendCount;
		}
		//	第二步， 发送到写位置的数据
		nResult = send(m_Socket.GetSocket(), pBufStart, nWriteIndex, 0);
		if (nResult == -1)
		{
			if (errno == EAGAIN)
			{
				m_bSendable = false;
				return 1;
			}
			else		//	出现其它错误，显示为连接断开，重新连接
			{
				SetIsConnnectted(0);

				close(m_Socket.GetSocket());
				m_Socket.SetSocket(-1);
				InfoRecorder(LOG_TYPE_CONNECTOR, LOG_LEVEL_ERROR, LOG_SUBTYPE_LOGIC, 1, __FILE__, "LConnector::SendData", __LINE__,
																												"3 send Failed, SystemErrorCode:%d\n", errno);
				return -1;
			}
		}
		else
		{
			m_FixLenCircleBufForSend.SetReadIndex(nWriteIndex);
			nSendCount += nResult;
		}
		return nSendCount;
	}
}

void LConnector::SetLogTypeAndLevel(unsigned int unLogType, unsigned int unLevel)
{
	if (m_pInfoRecorderBase == NULL)
	{
		return ;
	}
	m_pInfoRecorderBase->SetLogType(unLogType);
	m_pInfoRecorderBase->SetLogLevel(unLevel);
}
void LConnector::GetLogTypeAndLevel(unsigned int& unLogType, unsigned int& unLevel)
{
	if (m_pInfoRecorderBase == NULL)
	{
		return ;
	}
	unLogType	= m_pInfoRecorderBase->GetLogType();
	unLevel		= m_pInfoRecorderBase->GetLogLevel();
}

void LConnector::SetInfoRecorderBase(LInfoRecorderBase* pInfoRecorderBase)
{
	m_pInfoRecorderBase = pInfoRecorderBase;
}
void LConnector::InfoRecorder(unsigned int unLogType, unsigned int unLevel, unsigned int unLogSubType, unsigned int unThreadID, const char* pszFileName, const char* pszFunctionName, int nLine, const char* pszFmt, ...)
{
	if (m_pInfoRecorderBase == NULL)
	{
		return ;
	}
	va_list ap;
	va_start(ap, pszFmt);
	m_pInfoRecorderBase->InfoRecorder(unLogType, unLevel, unLogSubType, unThreadID, pszFileName, pszFunctionName, nLine, pszFmt, ap);
	va_end(ap);
}
//	=============================================
//

LConnectorConfigProcessor::LConnectorConfigProcessor()
{
	memset(&m_Cip, 0, sizeof(m_Cip));
	m_Cip.unReConnectPeriodTime = 10;
}

LConnectorConfigProcessor::~LConnectorConfigProcessor()
{
}

bool LConnectorConfigProcessor::Initialize(char* pConfigFileName, char* pSectionHeader, bool bReadIpAndPort)
{
	if (pConfigFileName == NULL || pSectionHeader == NULL || strlen(pSectionHeader) >= 64)
	{
		return false;
	}
	LIniFileReadAndWrite ifraw;
	if (!ifraw.OpenIniFile(pConfigFileName))
	{
		return false;
	}

	//	读取连接IP和端口
	char szSection[128 + 1];	
	memset(szSection, 0, sizeof(szSection));
	sprintf(szSection, "%s_Connector_Global", pSectionHeader);
	char* pSection = szSection;
	const char* pKey = NULL;

	if (bReadIpAndPort)
	{
		pKey = "IP";
		if (!ifraw.read_profile_string(pSection, pKey, m_Cip.szIP, sizeof(m_Cip.szIP) - 1, ""))
		{
			return false;
		}

		pKey = "PORT";
		int nPort = ifraw.read_profile_int(pSection, pKey, 0);
		if (nPort <= 0)
		{
			return false;
		}
		m_Cip.usPort = nPort;
	}

	pKey = "ReConnectPeriodTime";		//断开后的重连时间
	int nReConnectPeriodTime = ifraw.read_profile_int(pSection, pKey, 0);
	if (nReConnectPeriodTime <= 0)
	{
		return false;
	}
	m_Cip.unReConnectPeriodTime = nReConnectPeriodTime;
	
	pKey = "MaxPacketLen";		//	最大数据包
	int nMaxPacketLen = ifraw.read_profile_int(pSection, pKey, 0);
	if (nMaxPacketLen <= 0)
	{
		return false;
	}
	m_Cip.unMaxPacketLen = nMaxPacketLen;

	pKey = "PacketRecvBufLen";		//	最大数据包
	int nPacketRecvBufLen = ifraw.read_profile_int(pSection, pKey, 0);
	if (nPacketRecvBufLen <= 0)
	{
		return false;
	}
	m_Cip.unMaxRecvBufLen = nPacketRecvBufLen;

	pKey = "PacketRecvedPacketBufLen";		//	所有的接收的数据包的缓存长度
	int nPacketRecvedPacketBufLen = ifraw.read_profile_int(pSection, pKey, 0);
	if (nPacketRecvedPacketBufLen <= 0)
	{
		return false;
	}
	m_Cip.unRecvedPacketBufLen = nPacketRecvedPacketBufLen;

	pKey = "SendPacketBufLen";		//	发送数据缓存长度
	int nSendPacketBufLen = ifraw.read_profile_int(pSection, pKey, 0);
	if (nSendPacketBufLen <= 0)
	{
		return false;
	}
	m_Cip.unMaxSendBufLen = nSendPacketBufLen;

	return true;
}
t_Connector_Initialize_Params& LConnectorConfigProcessor::GetConnectorInitializeParams()
{
	return m_Cip;
}


