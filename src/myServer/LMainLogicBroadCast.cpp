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

#include "LMainLogicBroadCast.h"
#include "LNetWorkServices.h"	
#include <sys/prctl.h>
#include "LPacketSingle.h"
#include "LMyErrorRecorder.h"
#include "testpacket.h"

using namespace ns_testpacket;

#define MAX_SEND_PACKET_LEN 1000
LMainLogicBroadCast::LMainLogicBroadCast()
{
 	m_tLastWriteTime = 0;
	m_nSendPacketFreeCount = 0;
	m_nLastBroadCastID	= 0;
	m_u64PacketProcessed = 0;
}
LMainLogicBroadCast::~LMainLogicBroadCast()
{

}

bool LMainLogicBroadCast::Initialize(char* pszConfigFile)
{
	if (pszConfigFile == NULL)
	{
		return false;
	}

	if (!this->InitializeNetWork(pszConfigFile, 500, "Test", true))
	{
		return false;
	}
	LInfoRecorderBase* pInfoRecorderBase = new LInfoRecorderBase();
	LMyErrorRecorder* mEr =  new LMyErrorRecorder();
	pInfoRecorderBase->SetWriter(mEr);
	this->SetInfoRecorderBase(pInfoRecorderBase);
	this->SetLogTypeAndLevel(0xFFFFFFFF, LOG_LEVEL_DEBUG);
	return true;
}


bool LMainLogicBroadCast::OnAddSession(uint64_t u64SessionID, char* pszRemoteIP, unsigned short usRemotePort, int nRecvThreadID, int nSendThreadID)
{
	t_Session_Info tSessionInfo;
	tSessionInfo.u64SessionID 		= u64SessionID;
	tSessionInfo.unSendThreadID 	= nSendThreadID;
	tSessionInfo.nLastRecvPacketID = 1;


	map<uint64_t, t_Session_Info>::iterator _ito = m_mapSessionManaged.find(u64SessionID);
	if (_ito != m_mapSessionManaged.end())
	{
		return false;
	}
	m_mapSessionManaged[u64SessionID] = tSessionInfo;
	return true;
}
void LMainLogicBroadCast::OnRemoveSession(uint64_t u64SessionID)
{ 
	map<uint64_t, t_Session_Info>::iterator _ito = m_mapSessionManaged.find(u64SessionID);
	if (_ito != m_mapSessionManaged.end())
	{
		m_mapSessionManaged.erase(_ito);
	}
}
void LMainLogicBroadCast::OnRecvedPacket(uint64_t u64SessionID, const char* pPacketData, unsigned int unPacketDataLen)
{
	stpacketID stPacketDefined;
	int nUnSerializeResult = stPacketDefined.UnSerialize(const_cast<char*>(pPacketData), unPacketDataLen);
	if (nUnSerializeResult < 0)
	{
		return ;
	}
	int nPacketID = stPacketDefined.GetSingleValue_OrginType_npacketid();
	if (nPacketID == 1)
	{
		testpacketsingletype tpst;
		nUnSerializeResult = tpst.UnSerialize(const_cast<char*>(pPacketData), unPacketDataLen);
		if (nUnSerializeResult > 0)
		{
			printf("receive packet, id 1\n");
			testpacketsingletype tpst2Send;
			tpst2Send.SetSingleValue_OrginType_fValue(-10.0f);
			tpst2Send.SetSingleValue_OrginType_n64Value(23432426ll);
			tpst2Send.SetSingleValue_OrginType_nID(2);
			tpst2Send.SetSingleValue_OrginType_sValue(-112);
			tpst2Send.SetSingleValue_OrginType_testchar('o');
			tpst2Send.SetSingleValue_OrginType_un64Value(324242425234ll);
			stpacketID packetIDtoSend;
			packetIDtoSend.SetSingleValue_OrginType_npacketid(2);
			tpst2Send.SetSingleValue_UserDefineType_packetID(packetIDtoSend);

			char szDataToSend[1024 * 2];
			int nSerialize = tpst2Send.Serialize(szDataToSend, 1024 * 2);
			if (nSerialize > 0)
			{
				map<uint64_t, t_Session_Info>::iterator _itoSession = m_mapSessionManaged.find(u64SessionID);
				if (_itoSession == m_mapSessionManaged.end())
				{
					printf("Error Session Not Finded!!!, SessionID:%llu\n", u64SessionID);
					return ;
				}
				if (!SendOnePacket(_itoSession->first, szDataToSend, nSerialize))
				{
					char szError[512];
					sprintf(szError, "LMainLogicBroadCast::ThreadDoing, SendPacket Failed\n");
					printf("%s\n", szError);
				}
			}
		}
	}
//	m_u64PacketProcessed++;
//	LPacketSingle lGettenPacket(10 * 1024);
//	lGettenPacket.AddData((char*)pPacketData, (unsigned short)unPacketDataLen);
//
//	if (!lGettenPacket.CheckCRC32Code())
//	{
//		//printf("error Received Packet CRC32Code, Error, sessionID:%llu\n", u64SessionID);
//	}
//
//#if PRINT_INFO_TO_DEBUG
//	LSession* pSession = this->GetNetWorkServices()->GetSessionManager().FindSession(u64SessionID);
//	if (pSession != NULL)
//	{
//		pSession->AddPacketProcessed(1);
//	}
//#endif
//
//	char cPacketType = 0;
//	lGettenPacket.GetChar(cPacketType);
//
//	int nLastSendIndex = 0;
//	lGettenPacket.GetInt(nLastSendIndex);
//
//	//printf("RecvedData:%s\n", lGettenPacket.GetDataBuf() + 7);
//	map<uint64_t, t_Session_Info>::iterator _itoSession = m_mapSessionManaged.find(u64SessionID);
//	if (_itoSession == m_mapSessionManaged.end())
//	{
//		printf("Error Session Not Finded!!!, SessionID:%llu\n", u64SessionID);
//		return ;
//	}
//	if (_itoSession->second.nLastRecvPacketID != nLastSendIndex)
//	{
//		printf("recvIndex != ClientSendIndex!!!, ClientLastSendIndex:%d, LastRecvedIndex:%d\n", _itoSession->second.nLastRecvPacketID, nLastSendIndex);
//	}
//	_itoSession->second.nLastRecvPacketID++;
//
//	unsigned int unSendCount = 0;
//	if (random() % 10 > 11)
//	{
//		unsigned short usPacketLen = 11 + random() % MAX_SEND_PACKET_LEN;
//
//		LPacketSingle packetToSend(usPacketLen);
//		BuildRandomPacket(&packetToSend, true, m_nLastBroadCastID);
//		map<uint64_t, t_Session_Info>::iterator _ito = m_mapSessionManaged.begin();
//		while(_ito != m_mapSessionManaged.end())
//		{
//			if (!SendOnePacket(_ito->first, packetToSend.GetDataBuf(), packetToSend.GetDataLen()))
//			{
//				char szError[512];
//				sprintf(szError, "LMainLogicBroadCast::ThreadDoing, SendPacket Failed\n");
//				printf("%s\n", szError);
//			}
//			_ito++;
//		}
//
//
//		usPacketLen = 11 + random() % 100;
//		LPacketSingle packetToSend11(usPacketLen);
//		BuildRandomPacket(&packetToSend11, false, nLastSendIndex);
//		//printf("Send:%s\n", packetToSend.GetDataBuf() + 7);
//		if (!SendOnePacket(_itoSession->first, packetToSend11.GetDataBuf(), packetToSend11.GetDataLen()))
//		{
//
//		}
//		unSendCount++;
//	}
//	else
//	{
//		unsigned short usPacketLen = 11 + random() % MAX_SEND_PACKET_LEN;
//
//		LPacketSingle packetToSend(usPacketLen);
//		BuildRandomPacket(&packetToSend, false, nLastSendIndex);
//		//printf("Send:%s\n", packetToSend.GetDataBuf() + 7);
//		if (!SendOnePacket(_itoSession->first, packetToSend.GetDataBuf(), packetToSend.GetDataLen()))
//		{
//
//		}
//		unSendCount++;
//	}
}

int LMainLogicBroadCast::ThreadDoing(void* pParam)
{
	char szThreadName[128];
	sprintf(szThreadName, "MainLogicBroadCastThread");
	prctl(PR_SET_NAME, szThreadName);

	time_t tTimeToPrint = 0;
	while(1)
	{
		NetWorkServicesProcess(500);
		if (GetProcessedCount() == 0)
		{
			//	sched_yield();
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}

		if (CheckForStop())
		{
			break;
		}
		time_t tNow = time(NULL);
		if (tTimeToPrint == 0 || tNow - tTimeToPrint > 15)
		{
			//	NetWork __EPOLL_TEST_STATISTIC__
			printf("SessionCount:%d,PacketProcessed:%llu,", m_mapSessionManaged.size(), m_u64PacketProcessed);
			//this->GetNetWorkServices()->PrintfAllocAndFreeCount();
			tTimeToPrint = tNow;
			this->GetNetWorkServices()->KickOutIdleSession();

			this->GetNetWorkServices()->GetSessionManager().PrintAllSessionInfos();
			this->GetNetWorkServices()->PrintManagerInfos();
			//this->GetNetWorkServices()->PrintRefCountInfoForAll();
		}
	} 
	return 0;
}
bool LMainLogicBroadCast::OnStart()
{
	this->NetWorkStart();
	return true;
}
void LMainLogicBroadCast::OnStop()
{
	this->NetWorkDown();
}


bool LMainLogicBroadCast::BuildRandomPacket(LPacketSingle* pPacket, bool bBroadCast, int& nPacketSendIndex)
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
