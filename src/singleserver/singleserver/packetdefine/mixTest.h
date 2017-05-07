#pragma once



#include "OrignFile.h"


#include <vector>
using namespace std;

namespace packetdefine
{
	class stpacketid
	{
	private:
		int packetid;		//packetid
	public:
		stpacketid()
		{
			packetid = 0;
		}
		~stpacketid()
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
			//	packetid
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &packetid, unCurrentDataLen);
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
			//	packetid
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&packetid, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			return nCurrentUnSerializeLen;
		}
		//	packetid
		void SetSingleValue_OrginType_packetid(int vValue)
		{
			packetid = vValue;
		}
		int GetSingleValue_OrginType_packetid()
		{
			return packetid;
		}
	};

	class mixstructtest
	{
	private:
		stpacketid packetID;
		int nname;
		float fvalue;
		int nfixintArray[10];
		vector<float> vecfvarfloatArray;
		OrignTypeStruct ots;
		OrignTypeStruct otsfixArray[2];
		vector<OrignTypeStruct> vecotsvarArray;
	public:
		mixstructtest()
		{
			nname = 1;
			fvalue = 5.0f;
			memset(nfixintArray, 0, sizeof(nfixintArray));
			vecfvarfloatArray.clear();
			vecotsvarArray.clear();
		}
		~mixstructtest()
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

			//	nname
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &nname, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	fvalue
			unCurrentDataLen = sizeof(float);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &fvalue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	nfixintArray
			if (10 * sizeof(int) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, nfixintArray, 10 * sizeof(int));
			nCurrentSerializeLen += 10 * sizeof(int);

			//	fvarfloatArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_fvarfloatArray = (unsigned short)vecfvarfloatArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_fvarfloatArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_fvarfloatArray * sizeof(float) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_fvarfloatArray; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecfvarfloatArray[usIndex], sizeof(float));
				nCurrentSerializeLen += sizeof(float);
			}

			//	ots
			nStructSerializeResult = ots.Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen);
			if (nStructSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentSerializeLen += nStructSerializeResult;
			}

			//	otsfixArray
			for (unsigned short usIndex = 0; usIndex < 2; ++usIndex)
			{
				nStructSerializeResult = otsfixArray[usIndex].Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen); 													   
				if (nStructSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					nCurrentSerializeLen += nStructSerializeResult;
				}
			}

			//	otsvarArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_otsvarArray = (unsigned short)vecotsvarArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_otsvarArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_otsvarArray; ++usIndex) 													   
			{
				nStructSerializeResult = vecotsvarArray[usIndex].Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen); 													   
				if (nStructSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					nCurrentSerializeLen += nStructSerializeResult;
				}
			}

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

			//	nname
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&nname, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	fvalue
			unCurrentDataLen = sizeof(float);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&fvalue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	nfixintArray
			if (10 * sizeof(int) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(nfixintArray, pszBuf + nCurrentUnSerializeLen, 10 * sizeof(int));
			nCurrentUnSerializeLen += 10 * sizeof(int);

			//	fvarfloatArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_fvarfloatArray = 0;
			memcpy(&usDataItemCount_fvarfloatArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_fvarfloatArray * sizeof(float) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			float vTempData_fvarfloatArray;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_fvarfloatArray; ++usIndex)
			{
				memcpy(&vTempData_fvarfloatArray, pszBuf + nCurrentUnSerializeLen, sizeof(float));
				vecfvarfloatArray.push_back(vTempData_fvarfloatArray);
				nCurrentUnSerializeLen += sizeof(float);
			}

			//	ots
			nStructUnSerializeResult = ots.UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen);
			if (nStructUnSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentUnSerializeLen += nStructUnSerializeResult;
			}

			//	otsfixArray
			for (unsigned int unIndex = 0; unIndex < 2; ++unIndex)
			{
				nStructUnSerializeResult = otsfixArray[unIndex].UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen); 													   
				if (nStructUnSerializeResult < 0)
				{
					return -1;
				}else
				{
					nCurrentUnSerializeLen += nStructUnSerializeResult;
				}
			}

			//	otsvarArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_otsvarArray = 0;
			memcpy(&usDataItemCount_otsvarArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_otsvarArray; ++usIndex) 													   
			{
				OrignTypeStruct classTempData_otsvarArray;
				nStructUnSerializeResult = classTempData_otsvarArray.UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen); 													   
				if (nStructUnSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					vecotsvarArray.push_back(classTempData_otsvarArray);
					nCurrentUnSerializeLen += nStructUnSerializeResult;
				}
			}

			return nCurrentUnSerializeLen;
		}
		//	packetID
		void SetSingleValue_UserDefineType_packetID(stpacketid& vValue)
		{
			packetID = vValue;
		}
		void GetSingleValue_UserDefineType_packetID(stpacketid& vValue)
		{
			vValue = packetID;
		}
		//	nname
		void SetSingleValue_OrginType_nname(int vValue)
		{
			nname = vValue;
		}
		int GetSingleValue_OrginType_nname()
		{
			return nname;
		}
		//	fvalue
		void SetSingleValue_OrginType_fvalue(float vValue)
		{
			fvalue = vValue;
		}
		float GetSingleValue_OrginType_fvalue()
		{
			return fvalue;
		}
		//	nfixintArray
		void SetFixArray_OrginType_nfixintArray(int stArrayValue[10])
		{
			for (int i = 0; i < 10; ++i)
			{
				nfixintArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_nfixintArray(int stArrayValue[10])
		{
			for (int i = 0; i < 10; ++i)
			{
				stArrayValue[i] = nfixintArray[i];
			}
		}
		//	fvarfloatArray
		void AddToVarArray_OrginType_fvarfloatArray(float vArrayValue)
		{
			vecfvarfloatArray.push_back(vArrayValue);
		}
		vector<float>& GetVarArray_OrginType_fvarfloatArray()
		{
			return vecfvarfloatArray;
		}
		//	ots
		void SetSingleValue_UserDefineType_ots(OrignTypeStruct& vValue)
		{
			ots = vValue;
		}
		void GetSingleValue_UserDefineType_ots(OrignTypeStruct& vValue)
		{
			vValue = ots;
		}
		//	otsfixArray
		void SetFixArray_UserDefineType_otsfixArray(OrignTypeStruct stArrayValue[2])
		{
			for (int i = 0; i < 2; ++i)
			{
				otsfixArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_UserDefineType_otsfixArray(OrignTypeStruct stArrayValue[2])
		{
			for (int i = 0; i < 2; ++i)
			{
				stArrayValue[i] = otsfixArray[i];
			}
		}
		//	otsvarArray
		void AddToVarArray_UserDefineType_otsvarArray(OrignTypeStruct& vArrayValue)
		{
			vecotsvarArray.push_back(vArrayValue);
		}
		vector<OrignTypeStruct>& GetVarArray_UserDefineType_otsvarArray()
		{
			return vecotsvarArray;
		}
	};


}