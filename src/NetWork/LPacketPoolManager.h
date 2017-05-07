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

#ifndef __LINUX_PACKET_POOL_MANAGER_HEADER_INCLUDED_DEFINED__
#define __LINUX_PACKET_POOL_MANAGER_HEADER_INCLUDED_DEFINED__

#include "LPacketBase.h"
#include "LFixLenCircleBuf.h"


typedef struct _Packet_Pool_Desc
{
	unsigned short usPacketLen;
	unsigned int unInitSize;
	unsigned int unMaxAllocSize; 
	//_Packet_Pool_Desc()
	//{ 
	//	usPacketLen		= 0;
	//	unInitSize 		= 0;
	//	unMaxAllocSize 	= 0; 
	//}
}t_Packet_Pool_Desc;

template<class T> class LPacketPoolManager
{
public:
	LPacketPoolManager()
	{ 
		m_usMinPacketSize 		= 0;
		m_usMaxPacketSize 		= 0;
		m_unPacketLenTypeCount 	= 0;
		m_parrPacketPoolDesc 	= NULL;
		m_parrFixLenCircleBuf 	= NULL;
		m_pGlobalPoolManager 	= NULL;
		m_bEnableAllocBuf 		= false;
	}
	~LPacketPoolManager()
	{
	}

	void ReleasePacketPoolManagerResource()
	{ 
		if (m_parrPacketPoolDesc != NULL)
		{
			delete[] m_parrPacketPoolDesc;
		}
		if (m_parrFixLenCircleBuf != NULL)
		{
			for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
			{
				T* p = NULL;
				while (1)
				{
					E_Circle_Error error = m_parrFixLenCircleBuf[unIndex].GetOneItem((char*)&p, sizeof(T*));
					if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
					{
						break;
					}
					else
					{
						if (p->DecrementRefCountAndResultIsTrue())
						{
							delete p;
						}
						p = NULL;
					}
				}
			}
			delete[] m_parrFixLenCircleBuf;
		}
	}

	void SetReuqestPoolFromGlobalManager(LPacketPoolManager<T>* pGlobalPoolManager)
	{
		m_pGlobalPoolManager = pGlobalPoolManager;
	}
	void SetEnableAlloc(bool bEnableAllocBuf)
	{
		m_bEnableAllocBuf = bEnableAllocBuf;
	}
public:
	int GetCurrentContentCount()
	{
		int nCurrentCount = 0;
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			nCurrentCount += m_parrFixLenCircleBuf[unIndex].GetCurrentExistCount();
		}
		return nCurrentCount;
	}
	bool Initialize(t_Packet_Pool_Desc tpacketDesc[], size_t sArraySize)
	{
		if (sArraySize == 0)
		{
			return false;
		}
		if (OrderAndSetPacketDesc(tpacketDesc, sArraySize) == false)
		{
			return false;
		}
		m_parrFixLenCircleBuf = new LFixLenCircleBuf[m_unPacketLenTypeCount];
		if (m_parrFixLenCircleBuf == NULL)
		{
			return false;
		}
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			if (!m_parrFixLenCircleBuf[unIndex].Initialize(sizeof(T*), m_parrPacketPoolDesc[unIndex].unMaxAllocSize))
			{
				return false;
			}
			for (unsigned int unInnerIndex = 0; unInnerIndex < m_parrPacketPoolDesc[unIndex].unInitSize; ++unInnerIndex)
			{
				T* pTemp = new T(m_parrPacketPoolDesc[unIndex].usPacketLen);
				if (pTemp == NULL)
				{
					return false;
				}
				if (m_parrFixLenCircleBuf[unIndex].AddItems((char*)&pTemp, 1) != 0)
				{
					//	当前数据没有加入队列，那么需要释放该数据
					delete pTemp;
					return false;
				}
			}
		}
		return true;
	}

	T* RequestOnePacket(unsigned short usPacketLen)
	{
		T* pTemp = NULL;
		int nSelectedIndex = SelectIndexForPacketLen(usPacketLen);
		if (nSelectedIndex == -1)
		{
			return NULL;
		}

		if (m_parrFixLenCircleBuf[nSelectedIndex].GetOneItem((char*)&pTemp, sizeof(T*)) == E_Circle_Buf_No_Error)
		{
			return pTemp;
		}

		return pTemp;
	}
	//	其他管理器向本管理器请求部分数据
	int Request(LFixLenCircleBuf* pFixLenCircleBuf, unsigned int unExpecttedCount, unsigned short usPoolItemLen)
	{
		if (pFixLenCircleBuf == NULL)
		{
			return 0;
		}
		if (unExpecttedCount == 0)
		{
			return 0;
		}
		int nSelectedIndex = SelectIndexForPacketLenEqual(usPoolItemLen);
		if (nSelectedIndex == -1)
		{
			return 0;
		}
		int nGettedCount = m_parrFixLenCircleBuf[nSelectedIndex].RequestItems(pFixLenCircleBuf, unExpecttedCount);
		return nGettedCount;
	}
	bool FreeOneItemToPool(T* pValue, unsigned short usExtractBufLen)
	{
		int nSelectIndex = SelectIndexForPacketLenEqual(usExtractBufLen);
		if (nSelectIndex < 0)
		{
			return false;
		}
		if (m_parrFixLenCircleBuf[nSelectIndex].AddItems((char*)&pValue, 1) != E_Circle_Buf_No_Error)
		{
			return false;
		}
		return true;
	}
	bool CommitToAnother(LPacketPoolManager<T> *pPool, unsigned short usPoolSize)
	{
		if (pPool == NULL)
		{
			return false;
		}
		int nSelectIndex = SelectIndexForPacketLenEqual(usPoolSize);
		if (nSelectIndex < 0)
		{
			return false;
		}
		unsigned int unCanAddMaxItemCountToIt = 0;
		LFixLenCircleBuf* pDestBuf = pPool->GetFixLenCircleBuf(usPoolSize, unCanAddMaxItemCountToIt);
		if (pDestBuf == NULL)
		{
			return false;
		}
		if (!m_parrFixLenCircleBuf[nSelectIndex].CopyAllItemsToOtherFixLenCircleBuf(pDestBuf))
		{
			return false;
		}
		return true;
	}
	LFixLenCircleBuf* GetFixLenCircleBuf(unsigned short usPoolSize, unsigned int& unMaxCanAddtoPoolCount)
	{ 
		int nSelectIndex = SelectIndexForPacketLenEqual(usPoolSize);
		if (nSelectIndex == -1)
		{
			return NULL;
		}
		unMaxCanAddtoPoolCount = m_parrPacketPoolDesc[nSelectIndex].unMaxAllocSize - 1;
		return &m_parrFixLenCircleBuf[nSelectIndex];
	}
protected:
	bool OrderAndSetPacketDesc(t_Packet_Pool_Desc tPacketDesc[], size_t sArraySize)
	{
		unsigned int unArraySize = sArraySize;
		if (unArraySize == 0)
		{
			return false;
		}

		for (unsigned int unIndex = 0; unIndex < unArraySize; ++unIndex)
		{
			if (tPacketDesc[unIndex].usPacketLen == 0 || tPacketDesc[unIndex].unMaxAllocSize == 0)
			{
				return false;
			}
		}
		//	存在重复项，返回false
		for (unsigned int unIndex = 0; unIndex < unArraySize; ++unIndex)
		{
			for (unsigned int unInnerIndex = unIndex + 1; unInnerIndex < unArraySize; ++unInnerIndex)
			{
				if (tPacketDesc[unIndex].usPacketLen == tPacketDesc[unInnerIndex].usPacketLen)
				{
					return false;
				}
			}
		}
		m_unPacketLenTypeCount = unArraySize;
		//	排序，小的在前面
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			for (unsigned int unInnerIndex = unIndex + 1; unInnerIndex < m_unPacketLenTypeCount; ++unInnerIndex)
			{
				if (tPacketDesc[unIndex].usPacketLen > tPacketDesc[unInnerIndex].usPacketLen)
				{
					t_Packet_Pool_Desc tppd 	= tPacketDesc[unIndex];
					tPacketDesc[unIndex] 		= tPacketDesc[unInnerIndex];
					tPacketDesc[unInnerIndex] 	= tPacketDesc[unIndex];
				}
			}
		}

		m_parrPacketPoolDesc = new t_Packet_Pool_Desc[m_unPacketLenTypeCount];
		if (m_parrPacketPoolDesc == NULL)
		{
			return false;
		}
		memmove(m_parrPacketPoolDesc, tPacketDesc, sizeof(t_Packet_Pool_Desc) * m_unPacketLenTypeCount);
		
		m_usMinPacketSize 		= m_parrPacketPoolDesc[0].usPacketLen;
		m_usMaxPacketSize 		= m_parrPacketPoolDesc[m_unPacketLenTypeCount - 1].usPacketLen;

		return true;
	}
	int SelectIndexForPacketLen(unsigned short usPacketLen)
	{
		if (usPacketLen > m_usMaxPacketSize)
		{
			return -1;
		}
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			if (usPacketLen <= m_parrPacketPoolDesc[unIndex].usPacketLen)
			{
				return unIndex;
			}
		}
		return -1;
	}
	int SelectIndexForPacketLenEqual(unsigned short usPacketLen)
	{
		if (usPacketLen > m_usMaxPacketSize)
		{
			return -1;
		}
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			if (usPacketLen == m_parrPacketPoolDesc[unIndex].usPacketLen)
			{
				return unIndex;
			}
		}
		return -1;

	}
public: 
	bool CommitAllLocalPacketPoolToGlobalPacketPool()
	{
		if (m_pGlobalPoolManager)
		{
			return false;
		}
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			if (!CommitToAnother(m_pGlobalPoolManager, m_parrPacketPoolDesc[unIndex].usPacketLen))
			{
			}
		}
		return true;
	}
private:
	unsigned int m_unPacketLenTypeCount;
	unsigned short m_usMinPacketSize;
	unsigned short m_usMaxPacketSize;
	t_Packet_Pool_Desc* m_parrPacketPoolDesc;
	LFixLenCircleBuf* m_parrFixLenCircleBuf;
	LPacketPoolManager<T>* m_pGlobalPoolManager;
	bool m_bEnableAllocBuf;
};

#endif

