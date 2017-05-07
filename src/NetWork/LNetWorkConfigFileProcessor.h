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

#pragma once

#include "LIniFileReadAndWrite.h"
#include "LPacketPoolManager.h"
#include "LNetWorkServices.h"

#define MAX_RECV_POOL_TYPE_SIZE 30
#define MAX_SEND_POOL_TYPE_SIZE 30

class LNetWorkConfigFileProcessor
{
public:
	LNetWorkConfigFileProcessor(void);
	~LNetWorkConfigFileProcessor(void);
public:
	bool ReadConfig(char* pFileName, char* pSectionHeader);
	t_NetWorkServices_Params& GetNetWorkServicesParams()
	{
		return m_NetWorkServicesParams;
	}
private:
	bool ReadServicesBase();
	bool ReadServicesSession();
	bool ReadServicesGlobalParam();
private:
	t_NetWorkServices_Params m_NetWorkServicesParams;
	LIniFileReadAndWrite m_Inifile;

private:
	char m_szSectionHeader[64 + 1];
	char m_szIp[64 + 1];
};
