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
//#include ""
//
//
#include "LPacketPoolManager.h"
#include "LNetWorkServices.h"
#include "LMainLogicBroadCast.h"
#include "LNetWorkConfigFileProcessor.h"
#include <termios.h>

static struct termios oldt;

void RestoreTerminalState()
{
	tcsetattr(0, TCSANOW, &oldt);
}

void EnableTerminalEnterString()
{
	struct termios newt;


	tcgetattr(0, &oldt);

	newt = oldt;

	newt.c_iflag &= ~(ICANON | ECHO);

	tcsetattr(0, TCSANOW, &newt);

	atexit(RestoreTerminalState);
}


int main(int nargc, char* argv[])
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

	EnableTerminalEnterString();




	char* pFileName = "123.txt";
	LMainLogicBroadCast lMainLogicThread;

	if (!lMainLogicThread.Initialize("NetWorkConfig.ini"))
	{
		return -2;
	}

	printf("lMainLogicThread.Initialize Success Step 2\n");
	if (!lMainLogicThread.Start())
	{
		return -3;
		printf("lMainLogicThread.Start Success Step 3\n");
	}

	printf("lNetWorkServices.Start Success Step 4 NetServices Started\n");

	int ch = 0;
	while (1)
	{
		ch = getchar();
		if (ch == 'Q')
		{
			lMainLogicThread.Stop();
			break;
		}
		printf("Enter char is:%c\n", ch);
	}
	pthread_join(lMainLogicThread.GetThreadHandle(), NULL);

	return 0;
}

