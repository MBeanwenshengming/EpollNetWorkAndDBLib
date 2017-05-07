#pragma once

#include <vector>
using namespace std;

namespace ns_testpacket
{
	class stpacketID
	{
	private:
		int npacketid;		//packetid
	public:
		stpacketID()
		{
			npacketid = 0;
		}
		~stpacketID()
		{
		}
		int Serialize(char* pszBuf, unsigned int unBufLen)
		{
			int nCurrentSerializeLen = 0;
			unsigned int unCurrentDataLen = 0;
			if (pszBuf == NULL || unBufLen == 0)
			{
				return -1;
			}
			//	npacketid
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &npacketid, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			return nCurrentSerializeLen;
		}
		int UnSerialize(char* pszBuf, unsigned int unBufLen)
		{
			int nCurrentUnSerializeLen = 0;
			unsigned int unCurrentDataLen = 0;
			if (pszBuf == NULL || unBufLen == 0)
			{
				return -1;
			}
			//	npacketid
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&npacketid, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			return nCurrentUnSerializeLen;
		}
		//	npacketid
		void SetSingleValue_OrginType_npacketid(int vValue)
		{
			npacketid = vValue;
		}
		int GetSingleValue_OrginType_npacketid()
		{
			return npacketid;
		}
	};

	class testpacketsingletype
	{
	private:
		stpacketID packetID;
		int nID;		//testID
		char testchar;
		float fValue;
		int64_t n64Value;
		uint64_t un64Value;
		short sValue;
	public:
		testpacketsingletype()
		{
			nID = 0;
			testchar = 255;
			fValue = 0.0f;
			n64Value = 0;
			un64Value = 0;
			sValue = 0;
		}
		~testpacketsingletype()
		{
		}
		int Serialize(char* pszBuf, unsigned int unBufLen)
		{
			int nCurrentSerializeLen = 0;
			unsigned int unCurrentDataLen = 0;
			int nStructSerializeResult = 0;
			if (pszBuf == NULL || unBufLen == 0)
			{
				return -1;
			}
			//	packetID
			nStructSerializeResult = packetID.Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen);
			if (nStructSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentSerializeLen += nStructSerializeResult;
			}

			//	nID
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &nID, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	testchar
			unCurrentDataLen = sizeof(char);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &testchar, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	fValue
			unCurrentDataLen = sizeof(float);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &fValue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	n64Value
			unCurrentDataLen = sizeof(int64_t);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &n64Value, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	un64Value
			unCurrentDataLen = sizeof(uint64_t);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &un64Value, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	sValue
			unCurrentDataLen = sizeof(short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &sValue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			return nCurrentSerializeLen;
		}
		int UnSerialize(char* pszBuf, unsigned int unBufLen)
		{
			int nCurrentUnSerializeLen = 0;
			unsigned int unCurrentDataLen = 0;
			int nStructUnSerializeResult = 0;
			if (pszBuf == NULL || unBufLen == 0)
			{
				return -1;
			}
			//	packetID
			nStructUnSerializeResult = packetID.UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen);
			if (nStructUnSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentUnSerializeLen += nStructUnSerializeResult;
			}

			//	nID
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&nID, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	testchar
			unCurrentDataLen = sizeof(char);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&testchar, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	fValue
			unCurrentDataLen = sizeof(float);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&fValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	n64Value
			unCurrentDataLen = sizeof(int64_t);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&n64Value, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	un64Value
			unCurrentDataLen = sizeof(uint64_t);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&un64Value, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	sValue
			unCurrentDataLen = sizeof(short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&sValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			return nCurrentUnSerializeLen;
		}
		//	packetID
		void SetSingleValue_UserDefineType_packetID(stpacketID& vValue)
		{
			packetID = vValue;
		}
		void GetSingleValue_UserDefineType_packetID(stpacketID& vValue)
		{
			vValue = packetID;
		}
		//	nID
		void SetSingleValue_OrginType_nID(int vValue)
		{
			nID = vValue;
		}
		int GetSingleValue_OrginType_nID()
		{
			return nID;
		}
		//	testchar
		void SetSingleValue_OrginType_testchar(char vValue)
		{
			testchar = vValue;
		}
		char GetSingleValue_OrginType_testchar()
		{
			return testchar;
		}
		//	fValue
		void SetSingleValue_OrginType_fValue(float vValue)
		{
			fValue = vValue;
		}
		float GetSingleValue_OrginType_fValue()
		{
			return fValue;
		}
		//	n64Value
		void SetSingleValue_OrginType_n64Value(int64_t vValue)
		{
			n64Value = vValue;
		}
		int64_t GetSingleValue_OrginType_n64Value()
		{
			return n64Value;
		}
		//	un64Value
		void SetSingleValue_OrginType_un64Value(uint64_t vValue)
		{
			un64Value = vValue;
		}
		uint64_t GetSingleValue_OrginType_un64Value()
		{
			return un64Value;
		}
		//	sValue
		void SetSingleValue_OrginType_sValue(short vValue)
		{
			sValue = vValue;
		}
		short GetSingleValue_OrginType_sValue()
		{
			return sValue;
		}
	};


}
