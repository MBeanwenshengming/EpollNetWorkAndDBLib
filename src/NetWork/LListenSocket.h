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

#ifndef __LINUX_LISTEN_SOCKET_HEADERDEFINED__
#define __LINUX_LISTEN_SOCKET_HEADERDEFINED__
#include "LSocket.h"

#define MAX_LISTEN_IP_LEN 15

class LListenSocket : public LSocket
{
public:
	LListenSocket();
	~LListenSocket();
public:
	int Initialized(const char* pszIp, unsigned short usListenPort, unsigned int unSystemRecvBufLen, unsigned int unSystemSendBufLen);
	int Listen(unsigned int unListenNum);

private:
	unsigned short m_usListenSocket; 
	char m_szIp[MAX_LISTEN_IP_LEN + 1];
};
#endif

