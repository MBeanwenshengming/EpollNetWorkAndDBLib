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

#include "LListenSocket.h"


LListenSocket::LListenSocket()
{
	m_usListenSocket = 0;
	memset(m_szIp, 0, sizeof(m_szIp));
}

LListenSocket::~LListenSocket()
{
}

int LListenSocket::Initialized(const char* pszIp, unsigned short usListenPort, unsigned int unSystemRecvBufLen, unsigned int unSystemSendBufLen)
{ 
	int nListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (nListenSocket == -1)
	{
		return -1;
	}
	if (!SetSocket(nListenSocket))
	{
		return -2;
	}


#ifdef _DEBUG
	int nTempRecvBufLen = 0;
	socklen_t nTempRecvLen = sizeof(nTempRecvLen);
	if (GetSockOpt(SOL_SOCKET, SO_RCVBUF, &nTempRecvBufLen, &nTempRecvLen) != 0)
	{
	}
#endif
	unSystemRecvBufLen += 32;
	if (SetSockOpt(SOL_SOCKET, SO_RCVBUF, &unSystemRecvBufLen, sizeof(unSystemRecvBufLen)) != 0)
	{
		return -3;
	}
#ifdef _DEBUG
	int nTempSendBufLen = 0;
	socklen_t nTempSendLen = sizeof(nTempSendBufLen);
	if (GetSockOpt(SOL_SOCKET, SO_SNDBUF, &nTempSendBufLen, &nTempSendLen) != 0)
	{
	}
#endif
	unSystemSendBufLen += 32;
	if (SetSockOpt(SOL_SOCKET, SO_SNDBUF, &unSystemSendBufLen, sizeof(unSystemSendBufLen)) != 0)
	{
		return -4;
	}

	//	设置套接字为非阻塞套接字
	int nSetNonBlockSuccess = fcntl(GetSocket(), F_SETFL, O_NONBLOCK);
	if (nSetNonBlockSuccess == -1)
	{
		return -5;
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(usListenPort);
	int nConvertSuccess = inet_aton(pszIp, &serverAddr.sin_addr);
	if (nConvertSuccess == 0)
	{
		return -6;
	} 

	int nBindSuccess = bind(GetSocket(), (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (nBindSuccess == -1)
	{
		return -7;
	} 

	m_usListenSocket = usListenPort;
	strncpy(m_szIp, pszIp, MAX_LISTEN_IP_LEN);
	return 0;
}

int LListenSocket::Listen(unsigned int unListenNum)
{
	if (unListenNum == 0)
	{
		return -1;
	}
	if (GetSocket() == -1)
	{
		return -2;
	}
	int nListenSuccess = listen(GetSocket(), unListenNum);
	if (nListenSuccess == -1)
	{
		return -3;
	}
	return 0;
}


