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

#include "LConnectorTest.h"
#include "LPacketSingle.h"
#include "stdio.h"
#include <string>
#include "LMyErrorRecorder.h"
#include <sys/prctl.h>

#define MAX_SEND_PACKET_LEN	100
LConnectorTest::LConnectorTest()
{
	memset(m_szGettedOnePacket, 0, sizeof(m_szGettedOnePacket));
	m_unGettedOnePacketLen = 0;
	m_tLastSendData = 0;

	m_nLastRecvIndex	= 1;
	m_nLastSendIndex	= 1;
	m_bSendable			= true;
}

LConnectorTest::~LConnectorTest()
{
}

bool LConnectorTest::Initialize(char* pConfigFileName)
{
	if (pConfigFileName == NULL)
	{
		return false;
	}
	if (!m_ConfigProcessor.Initialize(pConfigFileName, "Test", true))
	{
		return false;
	}

	LInfoRecorderBase* pInfoRecorderBase = new LInfoRecorderBase();
	LMyErrorRecorder* mEr =  new LMyErrorRecorder();
	pInfoRecorderBase->SetWriter(mEr);
	m_Connector.SetInfoRecorderBase(pInfoRecorderBase);
	m_Connector.SetLogTypeAndLevel(0xFFFFFFFF, LOG_LEVEL_DEBUG);

	t_Connector_Initialize_Params tcip = m_ConfigProcessor.GetConnectorInitializeParams();
	if (!m_Connector.Initialize(tcip.szIP, tcip.usPort, tcip.unReConnectPeriodTime, tcip.unRecvedPacketBufLen, tcip.unMaxSendBufLen, tcip.unMaxRecvBufLen, tcip.unMaxPacketLen))
	{
		return false;
	}

	if (!m_Connector.Start())
	{
		return false;
	}

	return true;
}

int LConnectorTest::ThreadDoing(void* pParam)
{
	char szThreadName[128];
	sprintf(szThreadName, "LConnectorTest_MainLogic");
	prctl(PR_SET_NAME, szThreadName);

	int nLastSendIndex = 1;
	int nLastRecvIndex = 0;
	int nPacketRecvedCount = 0;
	int nPacketSendCount = 0;
	int nBroadCastIndex = 0;

	while (1)
	{
		unsigned int unGettedOnePacketLen = 0;
		bool bGettedSuccessed = m_Connector.GetOneRecvedPacket(m_szGettedOnePacket, 64 * 1024, unGettedOnePacketLen);

		if (bGettedSuccessed)
		{
			m_Connector.m_nRecvedPacketsMainLogic++;
			LPacketSingle lPakcetSingle(20 * 1024);
			lPakcetSingle.DirectSetData(m_szGettedOnePacket, unGettedOnePacketLen);

			if (!lPakcetSingle.CheckCRC32Code())
			{
				printf("Packet Receive CrC32Code Error\n");
			}
			char cPacketType = 0;
			lPakcetSingle.GetChar(cPacketType);

			int nLastRecvIndex = 0;
			lPakcetSingle.GetInt(nLastRecvIndex);

			if (cPacketType == 0)
			{
				if (nLastRecvIndex != nLastSendIndex - 1)
				{
					printf("RecvIndex:%d, LastSendIndex:%d\n", nLastRecvIndex, nLastSendIndex);
				}
				else
				{
					m_bSendable = true;
				}
			}
			else if (cPacketType == 1)
			{
				if (nBroadCastIndex == 0)
				{
					nBroadCastIndex = nLastRecvIndex;
				}
				if (nLastRecvIndex != nBroadCastIndex)
				{
					printf("LastBroadCastRecvIndex:%d, LastSendIndex:%d, BroadCastIndex:%d\n", nLastRecvIndex, nLastSendIndex, nBroadCastIndex);
				}
				nBroadCastIndex++;
			}
			else
			{
				printf("ErrorPacketType\n");
			}
			nPacketRecvedCount++;
			//	处理接收的数据包
			//printf("Recved:%s\n", lPakcetSingle.GetDataBuf() + 7);
		}
		else
		{
			//sched_yield(); 
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		time_t tNow = time(NULL);
		//	查看是否需要发送数据包
		if (m_Connector.IsConnectted() && m_bSendable)
		{
			unsigned short usPacketLen = 11 + random() % MAX_SEND_PACKET_LEN;

			LPacketSingle packetToSend(usPacketLen);
			BuildRandomPacket(&packetToSend, false, nLastSendIndex);
			//printf("send:%s\n", packetToSend.GetDataBuf() + 7);
			m_Connector.SendOnePacket(packetToSend.GetDataBuf(), packetToSend.GetDataLen());
			m_bSendable = false;
			nPacketSendCount++;
			//m_tLastSendData = tNow;
		}
		if (tNow - m_tLastSendData > 5)
		{
			printf("Sended:%d, Recved:%d\n", nPacketSendCount, nPacketRecvedCount);
			m_tLastSendData = tNow;
		}
	}
	return 0;
}
bool LConnectorTest::OnStart()
{
	return true;
}
void LConnectorTest::OnStop()
{
	m_Connector.Stop();
	return ;
}

bool LConnectorTest::BuildRandomPacket(LPacketSingle* pPacket, bool bBroadCast, int& nPacketSendIndex)
{
	if (pPacket == NULL)
	 {
	  return false;
	 }
	 unsigned short usReserveBytes = 4 + 2 + 1 + 4;
	 unsigned short usPacketBufLenAllBytes = pPacket->GetPacketBufLen();
	 if (usPacketBufLenAllBytes < usReserveBytes)
	 {
	  return false;
	 }
	 // 留下4字节填写CRC32码和2字节包头，1字节数据包类型(单播0，广播1)，4字节的数据包顺序
	 unsigned short usPacketRandomBufLen = pPacket->GetPacketBufLen() - usReserveBytes;
	 if (usPacketRandomBufLen >= 8000)
	 {
	  return false;
	 }

	 unsigned short usPacketLenVar = usPacketRandomBufLen;
	 char szPacketbuf[8000]; memset(szPacketbuf, 0, sizeof(szPacketbuf));
	 int nLen = 0;
	 while (usPacketLenVar--)
	 {
	  if (rand() % 2 == 0)
	  {
	   szPacketbuf[nLen] = 'a' + rand() % 26;
	  }
	  else
	  {
	   szPacketbuf[nLen] = 'A' + rand() % 26;
	  }
	  nLen++;
	 }

	 char cPacketType = bBroadCast ? 1 : 0;
	 int npacketID = nPacketSendIndex;

	 pPacket->AddChar(cPacketType);
	 pPacket->AddInt(npacketID);
	 pPacket->AddData(szPacketbuf, usPacketRandomBufLen);
	 pPacket->MakeCRC32CodeToPacket();
	 nPacketSendIndex++;

	 pPacket->CheckCRC32Code();
	 return true;
}
