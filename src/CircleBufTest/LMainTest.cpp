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

#include "IncludeHeader.h"
#include "LVarLenCircleBuf.h"

int main (int nargc, char* argv[])
{
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGPIPE, &action, NULL);

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGHUP, &action, NULL);

	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGPIPE);
	int n = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	if (n != 0)
	{
		return -1;
	}

	LVarLenCircleBuf myVarLenCircleBuf;
	myVarLenCircleBuf.InitializeVarLenCircleBuf(1024 * 1024);

	myVarLenCircleBuf.SetCurrentReadAndWriteIndex(1024 * 1024 - 3, 1024 * 1024 - 3);

	char szContent[100];
	strncpy(szContent, "abcdefghijklmnopqrstuvwxyz", 26);

	//myVarLenCircleBuf.AddData(szContent, 26);
	myVarLenCircleBuf.AddSessionIDAndData(12345, szContent, 26);

	memset(szContent, 0, sizeof(szContent));
	unsigned int unGettedDataLen = 0;
	myVarLenCircleBuf.GetData(szContent, 100, unGettedDataLen);
	uint64_t u64SessionID = *((uint64_t*)szContent);
	char* pData = szContent + 8;
	return 0;
}

