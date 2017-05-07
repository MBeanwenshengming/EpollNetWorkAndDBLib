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

#ifndef LVARLENCIRCLEBUF_H_
#define LVARLENCIRCLEBUF_H_

#include "LFixLenCircleBuf.h"

class LVarLenCircleBuf
{
public:
	LVarLenCircleBuf();
	~LVarLenCircleBuf();
public:
	//	初始化环形缓存区
	bool InitializeVarLenCircleBuf(unsigned int unTotalBufLen);
	//	添加一次数据
	//	pDataBuf 需要添加的数据的缓存
	//	unDataLen 要添加的数据的长度
	E_Circle_Error AddData(char* pDataBuf, unsigned int unDataLen);
	//	获取一次数据，成功获取后，数据写在pDataBuf缓存中
	//	pDataBuf 获取数据的缓存
	//	unDataBufLen 缓存的长度
	//	unGettedDataLen 获取的数据的长度
	E_Circle_Error GetData(char* pDataBuf, unsigned int unDataBufLen, unsigned int& unGettedDataLen);

	//	添加数据，在数据前面添加SessionID，主要是向接收数据包环形缓存中加入数据
	E_Circle_Error AddSessionIDAndData(uint64_t u64SessionID, char* pDataBuf, unsigned int unDataLen);
public:
	void ClearContent();
	int GetCurrentExistCount();
	void GetCurrentReadAndWriteIndex(int& nReadIndex, int& nWriteIndex);
	void SetCurrentReadAndWriteIndex(int nReadIndex, int nWriteIndex);
	int GetTotalBufLen();
private:
	volatile int m_ReadIndex;
	volatile int m_WriteIndex;
	char*	 m_pbuf;
	unsigned int m_unTotalBufLen;
};

#endif /* LVARLENCIRCLEBUF_H_ */
