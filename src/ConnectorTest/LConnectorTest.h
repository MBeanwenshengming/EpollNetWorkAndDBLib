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

#ifndef __LCONNECTOR_TEST_HEADER_DEFINED__
#define __LCONNECTOR_TEST_HEADER_DEFINED__

#include "LConnector.h"

class LPacketSingle;

class LConnectorTest : public LThreadBase
{
public:
	LConnectorTest();
	virtual ~LConnectorTest();
public:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();
public:
	bool Initialize(char* pConfigFileName);
public:
	bool BuildRandomPacket(LPacketSingle* pPacket, bool bBroadCast, int& nPacketSendIndex);
private:
	LConnector m_Connector;
	LConnectorConfigProcessor m_ConfigProcessor;

	time_t m_tLastSendData;	
	char m_szGettedOnePacket[64 * 1024];
	unsigned int m_unGettedOnePacketLen;
	int m_nLastRecvIndex;
	int m_nLastSendIndex;
	bool m_bSendable;
};

#endif


