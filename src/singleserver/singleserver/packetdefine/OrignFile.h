#pragma once

#include <vector>
using namespace std;


namespace packetdefine
{
	class OrignTypeStruct
	{
	private:

		int nValue;
		unsigned int unValue;
		float fValue;
		double dbValue;
		short sValue;
		unsigned short usValue;
		char cValue;
		unsigned char ucValue;
		int64_t n64Value;
		uint64_t un64Value;
	public:
		OrignTypeStruct()
		{
			nValue = 0;
			unValue = 0;
			fValue = 0.0f;
			dbValue = 0;
			sValue = 0;
			usValue = 0;
			cValue = 0;
			ucValue = 0;
			n64Value = 0;
			un64Value = 0;
		}
		~OrignTypeStruct()
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
			//	nValue
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &nValue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	unValue
			unCurrentDataLen = sizeof(unsigned int);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &unValue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	fValue
			unCurrentDataLen = sizeof(float);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &fValue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	dbValue
			unCurrentDataLen = sizeof(double);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &dbValue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	sValue
			unCurrentDataLen = sizeof(short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &sValue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	usValue
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &usValue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	cValue
			unCurrentDataLen = sizeof(char);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &cValue, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;

			//	ucValue
			unCurrentDataLen = sizeof(unsigned char);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, &ucValue, unCurrentDataLen);
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
			//	nValue
			unCurrentDataLen = sizeof(int);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&nValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	unValue
			unCurrentDataLen = sizeof(unsigned int);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&unValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	fValue
			unCurrentDataLen = sizeof(float);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&fValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	dbValue
			unCurrentDataLen = sizeof(double);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&dbValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	sValue
			unCurrentDataLen = sizeof(short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&sValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	usValue
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&usValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	cValue
			unCurrentDataLen = sizeof(char);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&cValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;

			//	ucValue
			unCurrentDataLen = sizeof(unsigned char);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(&ucValue, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
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

			return nCurrentUnSerializeLen;
		}
		//	nValue
		void SetSingleValue_OrginType_nValue(int vValue)
		{
			nValue = vValue;
		}
		int GetSingleValue_OrginType_nValue()
		{
			return nValue;
		}
		//	unValue
		void SetSingleValue_OrginType_unValue(unsigned int vValue)
		{
			unValue = vValue;
		}
		unsigned int GetSingleValue_OrginType_unValue()
		{
			return unValue;
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
		//	dbValue
		void SetSingleValue_OrginType_dbValue(double vValue)
		{
			dbValue = vValue;
		}
		double GetSingleValue_OrginType_dbValue()
		{
			return dbValue;
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
		//	usValue
		void SetSingleValue_OrginType_usValue(unsigned short vValue)
		{
			usValue = vValue;
		}
		unsigned short GetSingleValue_OrginType_usValue()
		{
			return usValue;
		}
		//	cValue
		void SetSingleValue_OrginType_cValue(char vValue)
		{
			cValue = vValue;
		}
		char GetSingleValue_OrginType_cValue()
		{
			return cValue;
		}
		//	ucValue
		void SetSingleValue_OrginType_ucValue(unsigned char vValue)
		{
			ucValue = vValue;
		}
		unsigned char GetSingleValue_OrginType_ucValue()
		{
			return ucValue;
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
	};

	class OrginTypeArrayFixLenStruct
	{
	private:
		int intArray[5];
		unsigned int uintArray[5];
		float floatArray[3];
		double doubleArray[10];
		short shortArray[5];
		unsigned short ushortArray[5];
		char charArray[5];
		unsigned char ucharArray[5];
		int64_t int64Array[5];
		uint64_t uint64Array[5];
	public:
		OrginTypeArrayFixLenStruct()
		{
			memset(intArray, 0, sizeof(intArray));
			memset(uintArray, 0, sizeof(uintArray));
			memset(floatArray, 0, sizeof(floatArray));
			memset(doubleArray, 0, sizeof(doubleArray));
			memset(shortArray, 0, sizeof(shortArray));
			memset(ushortArray, 0, sizeof(ushortArray));
			memset(charArray, 0, sizeof(charArray));
			memset(ucharArray, 0, sizeof(ucharArray));
			memset(int64Array, 0, sizeof(int64Array));
			memset(uint64Array, 0, sizeof(uint64Array));
		}
		~OrginTypeArrayFixLenStruct()
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
			//	intArray
			if (5 * sizeof(int) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, intArray, 5 * sizeof(int));
			nCurrentSerializeLen += 5 * sizeof(int);

			//	uintArray
			if (5 * sizeof(unsigned int) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, uintArray, 5 * sizeof(unsigned int));
			nCurrentSerializeLen += 5 * sizeof(unsigned int);

			//	floatArray
			if (3 * sizeof(float) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, floatArray, 3 * sizeof(float));
			nCurrentSerializeLen += 3 * sizeof(float);

			//	doubleArray
			if (10 * sizeof(double) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, doubleArray, 10 * sizeof(double));
			nCurrentSerializeLen += 10 * sizeof(double);

			//	shortArray
			if (5 * sizeof(short) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, shortArray, 5 * sizeof(short));
			nCurrentSerializeLen += 5 * sizeof(short);

			//	ushortArray
			if (5 * sizeof(unsigned short) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, ushortArray, 5 * sizeof(unsigned short));
			nCurrentSerializeLen += 5 * sizeof(unsigned short);

			//	charArray
			if (5 * sizeof(char) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, charArray, 5 * sizeof(char));
			nCurrentSerializeLen += 5 * sizeof(char);

			//	ucharArray
			if (5 * sizeof(unsigned char) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, ucharArray, 5 * sizeof(unsigned char));
			nCurrentSerializeLen += 5 * sizeof(unsigned char);

			//	int64Array
			if (5 * sizeof(int64_t) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, int64Array, 5 * sizeof(int64_t));
			nCurrentSerializeLen += 5 * sizeof(int64_t);

			//	uint64Array
			if (5 * sizeof(uint64_t) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			memcpy(pszBuf + nCurrentSerializeLen, uint64Array, 5 * sizeof(uint64_t));
			nCurrentSerializeLen += 5 * sizeof(uint64_t);

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
			//	intArray
			if (5 * sizeof(int) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(intArray, pszBuf + nCurrentUnSerializeLen, 5 * sizeof(int));
			nCurrentUnSerializeLen += 5 * sizeof(int);

			//	uintArray
			if (5 * sizeof(unsigned int) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(uintArray, pszBuf + nCurrentUnSerializeLen, 5 * sizeof(unsigned int));
			nCurrentUnSerializeLen += 5 * sizeof(unsigned int);

			//	floatArray
			if (3 * sizeof(float) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(floatArray, pszBuf + nCurrentUnSerializeLen, 3 * sizeof(float));
			nCurrentUnSerializeLen += 3 * sizeof(float);

			//	doubleArray
			if (10 * sizeof(double) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(doubleArray, pszBuf + nCurrentUnSerializeLen, 10 * sizeof(double));
			nCurrentUnSerializeLen += 10 * sizeof(double);

			//	shortArray
			if (5 * sizeof(short) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(shortArray, pszBuf + nCurrentUnSerializeLen, 5 * sizeof(short));
			nCurrentUnSerializeLen += 5 * sizeof(short);

			//	ushortArray
			if (5 * sizeof(unsigned short) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(ushortArray, pszBuf + nCurrentUnSerializeLen, 5 * sizeof(unsigned short));
			nCurrentUnSerializeLen += 5 * sizeof(unsigned short);

			//	charArray
			if (5 * sizeof(char) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(charArray, pszBuf + nCurrentUnSerializeLen, 5 * sizeof(char));
			nCurrentUnSerializeLen += 5 * sizeof(char);

			//	ucharArray
			if (5 * sizeof(unsigned char) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(ucharArray, pszBuf + nCurrentUnSerializeLen, 5 * sizeof(unsigned char));
			nCurrentUnSerializeLen += 5 * sizeof(unsigned char);

			//	int64Array
			if (5 * sizeof(int64_t) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(int64Array, pszBuf + nCurrentUnSerializeLen, 5 * sizeof(int64_t));
			nCurrentUnSerializeLen += 5 * sizeof(int64_t);

			//	uint64Array
			if (5 * sizeof(uint64_t) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			memcpy(uint64Array, pszBuf + nCurrentUnSerializeLen, 5 * sizeof(uint64_t));
			nCurrentUnSerializeLen += 5 * sizeof(uint64_t);

			return nCurrentUnSerializeLen;
		}
		//	intArray
		void SetFixArray_OrginType_intArray(int stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				intArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_intArray(int stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = intArray[i];
			}
		}
		//	uintArray
		void SetFixArray_OrginType_uintArray(unsigned int stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				uintArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_uintArray(unsigned int stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = uintArray[i];
			}
		}
		//	floatArray
		void SetFixArray_OrginType_floatArray(float stArrayValue[3])
		{
			for (int i = 0; i < 3; ++i)
			{
				floatArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_floatArray(float stArrayValue[3])
		{
			for (int i = 0; i < 3; ++i)
			{
				stArrayValue[i] = floatArray[i];
			}
		}
		//	doubleArray
		void SetFixArray_OrginType_doubleArray(double stArrayValue[10])
		{
			for (int i = 0; i < 10; ++i)
			{
				doubleArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_doubleArray(double stArrayValue[10])
		{
			for (int i = 0; i < 10; ++i)
			{
				stArrayValue[i] = doubleArray[i];
			}
		}
		//	shortArray
		void SetFixArray_OrginType_shortArray(short stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				shortArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_shortArray(short stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = shortArray[i];
			}
		}
		//	ushortArray
		void SetFixArray_OrginType_ushortArray(unsigned short stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				ushortArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_ushortArray(unsigned short stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = ushortArray[i];
			}
		}
		//	charArray
		void SetFixArray_OrginType_charArray(char stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				charArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_charArray(char stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = charArray[i];
			}
		}
		//	ucharArray
		void SetFixArray_OrginType_ucharArray(unsigned char stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				ucharArray[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_ucharArray(unsigned char stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = ucharArray[i];
			}
		}
		//	int64Array
		void SetFixArray_OrginType_int64Array(int64_t stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				int64Array[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_int64Array(int64_t stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = int64Array[i];
			}
		}
		//	uint64Array
		void SetFixArray_OrginType_uint64Array(uint64_t stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				uint64Array[i] = stArrayValue[i];
			}
		}
		void GetFixArray_OrginType_uint64Array(uint64_t stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = uint64Array[i];
			}
		}
	};

	class OrignTypeArrayVarLenStruct
	{
	private:
		vector<int> vecintArray;
		vector<unsigned int> vecuintArray;
		vector<float> vecfloatArray;
		vector<double> vecdoubleArray;
		vector<short> vecshortArray;
		vector<unsigned short> vecushortArray;
		vector<char> veccharArray;
		vector<unsigned char> vecucharArray;
		vector<int64_t> vecint64Array;
		vector<uint64_t> vecuint64Array;
	public:
		OrignTypeArrayVarLenStruct()
		{
			vecintArray.clear();
			vecuintArray.clear();
			vecfloatArray.clear();
			vecdoubleArray.clear();
			vecshortArray.clear();
			vecushortArray.clear();
			veccharArray.clear();
			vecucharArray.clear();
			vecint64Array.clear();
			vecuint64Array.clear();
		}
		~OrignTypeArrayVarLenStruct()
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
			//	intArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_intArray = (unsigned short)vecintArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_intArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_intArray * sizeof(int) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_intArray; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecintArray[usIndex], sizeof(int));
				nCurrentSerializeLen += sizeof(int);
			}

			//	uintArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_uintArray = (unsigned short)vecuintArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_uintArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_uintArray * sizeof(unsigned int) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_uintArray; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecuintArray[usIndex], sizeof(unsigned int));
				nCurrentSerializeLen += sizeof(unsigned int);
			}

			//	floatArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_floatArray = (unsigned short)vecfloatArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_floatArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_floatArray * sizeof(float) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_floatArray; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecfloatArray[usIndex], sizeof(float));
				nCurrentSerializeLen += sizeof(float);
			}

			//	doubleArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_doubleArray = (unsigned short)vecdoubleArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_doubleArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_doubleArray * sizeof(double) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_doubleArray; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecdoubleArray[usIndex], sizeof(double));
				nCurrentSerializeLen += sizeof(double);
			}

			//	shortArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_shortArray = (unsigned short)vecshortArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_shortArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_shortArray * sizeof(short) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_shortArray; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecshortArray[usIndex], sizeof(short));
				nCurrentSerializeLen += sizeof(short);
			}

			//	ushortArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_ushortArray = (unsigned short)vecushortArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_ushortArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_ushortArray * sizeof(unsigned short) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_ushortArray; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecushortArray[usIndex], sizeof(unsigned short));
				nCurrentSerializeLen += sizeof(unsigned short);
			}

			//	charArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_charArray = (unsigned short)veccharArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_charArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_charArray * sizeof(char) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_charArray; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &veccharArray[usIndex], sizeof(char));
				nCurrentSerializeLen += sizeof(char);
			}

			//	ucharArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_ucharArray = (unsigned short)vecucharArray.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_ucharArray, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_ucharArray * sizeof(unsigned char) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_ucharArray; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecucharArray[usIndex], sizeof(unsigned char));
				nCurrentSerializeLen += sizeof(unsigned char);
			}

			//	int64Array
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_int64Array = (unsigned short)vecint64Array.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_int64Array, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_int64Array * sizeof(int64_t) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_int64Array; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecint64Array[usIndex], sizeof(int64_t));
				nCurrentSerializeLen += sizeof(int64_t);
			}

			//	uint64Array
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_uint64Array = (unsigned short)vecuint64Array.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_uint64Array, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			if (usDataItemCount_uint64Array * sizeof(uint64_t) > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_uint64Array; ++usIndex)
			{
				memcpy(pszBuf + nCurrentSerializeLen, &vecuint64Array[usIndex], sizeof(uint64_t));
				nCurrentSerializeLen += sizeof(uint64_t);
			}

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
			//	intArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_intArray = 0;
			memcpy(&usDataItemCount_intArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_intArray * sizeof(int) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			int vTempData_intArray;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_intArray; ++usIndex)
			{
				memcpy(&vTempData_intArray, pszBuf + nCurrentUnSerializeLen, sizeof(int));
				vecintArray.push_back(vTempData_intArray);
				nCurrentUnSerializeLen += sizeof(int);
			}

			//	uintArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_uintArray = 0;
			memcpy(&usDataItemCount_uintArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_uintArray * sizeof(unsigned int) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned int vTempData_uintArray;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_uintArray; ++usIndex)
			{
				memcpy(&vTempData_uintArray, pszBuf + nCurrentUnSerializeLen, sizeof(unsigned int));
				vecuintArray.push_back(vTempData_uintArray);
				nCurrentUnSerializeLen += sizeof(unsigned int);
			}

			//	floatArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_floatArray = 0;
			memcpy(&usDataItemCount_floatArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_floatArray * sizeof(float) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			float vTempData_floatArray;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_floatArray; ++usIndex)
			{
				memcpy(&vTempData_floatArray, pszBuf + nCurrentUnSerializeLen, sizeof(float));
				vecfloatArray.push_back(vTempData_floatArray);
				nCurrentUnSerializeLen += sizeof(float);
			}

			//	doubleArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_doubleArray = 0;
			memcpy(&usDataItemCount_doubleArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_doubleArray * sizeof(double) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			double vTempData_doubleArray;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_doubleArray; ++usIndex)
			{
				memcpy(&vTempData_doubleArray, pszBuf + nCurrentUnSerializeLen, sizeof(double));
				vecdoubleArray.push_back(vTempData_doubleArray);
				nCurrentUnSerializeLen += sizeof(double);
			}

			//	shortArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_shortArray = 0;
			memcpy(&usDataItemCount_shortArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_shortArray * sizeof(short) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			short vTempData_shortArray;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_shortArray; ++usIndex)
			{
				memcpy(&vTempData_shortArray, pszBuf + nCurrentUnSerializeLen, sizeof(short));
				vecshortArray.push_back(vTempData_shortArray);
				nCurrentUnSerializeLen += sizeof(short);
			}

			//	ushortArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_ushortArray = 0;
			memcpy(&usDataItemCount_ushortArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_ushortArray * sizeof(unsigned short) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short vTempData_ushortArray;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_ushortArray; ++usIndex)
			{
				memcpy(&vTempData_ushortArray, pszBuf + nCurrentUnSerializeLen, sizeof(unsigned short));
				vecushortArray.push_back(vTempData_ushortArray);
				nCurrentUnSerializeLen += sizeof(unsigned short);
			}

			//	charArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_charArray = 0;
			memcpy(&usDataItemCount_charArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_charArray * sizeof(char) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			char vTempData_charArray;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_charArray; ++usIndex)
			{
				memcpy(&vTempData_charArray, pszBuf + nCurrentUnSerializeLen, sizeof(char));
				veccharArray.push_back(vTempData_charArray);
				nCurrentUnSerializeLen += sizeof(char);
			}

			//	ucharArray
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_ucharArray = 0;
			memcpy(&usDataItemCount_ucharArray, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_ucharArray * sizeof(unsigned char) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned char vTempData_ucharArray;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_ucharArray; ++usIndex)
			{
				memcpy(&vTempData_ucharArray, pszBuf + nCurrentUnSerializeLen, sizeof(unsigned char));
				vecucharArray.push_back(vTempData_ucharArray);
				nCurrentUnSerializeLen += sizeof(unsigned char);
			}

			//	int64Array
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_int64Array = 0;
			memcpy(&usDataItemCount_int64Array, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_int64Array * sizeof(int64_t) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			int64_t vTempData_int64Array;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_int64Array; ++usIndex)
			{
				memcpy(&vTempData_int64Array, pszBuf + nCurrentUnSerializeLen, sizeof(int64_t));
				vecint64Array.push_back(vTempData_int64Array);
				nCurrentUnSerializeLen += sizeof(int64_t);
			}

			//	uint64Array
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_uint64Array = 0;
			memcpy(&usDataItemCount_uint64Array, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			if (usDataItemCount_uint64Array * sizeof(uint64_t) + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			uint64_t vTempData_uint64Array;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_uint64Array; ++usIndex)
			{
				memcpy(&vTempData_uint64Array, pszBuf + nCurrentUnSerializeLen, sizeof(uint64_t));
				vecuint64Array.push_back(vTempData_uint64Array);
				nCurrentUnSerializeLen += sizeof(uint64_t);
			}

			return nCurrentUnSerializeLen;
		}
		//	intArray
		void AddToVarArray_OrginType_intArray(int vArrayValue)
		{
			vecintArray.push_back(vArrayValue);
		}
		vector<int>& GetVarArray_OrginType_intArray()
		{
			return vecintArray;
		}
		//	uintArray
		void AddToVarArray_OrginType_uintArray(unsigned int vArrayValue)
		{
			vecuintArray.push_back(vArrayValue);
		}
		vector<unsigned int>& GetVarArray_OrginType_uintArray()
		{
			return vecuintArray;
		}
		//	floatArray
		void AddToVarArray_OrginType_floatArray(float vArrayValue)
		{
			vecfloatArray.push_back(vArrayValue);
		}
		vector<float>& GetVarArray_OrginType_floatArray()
		{
			return vecfloatArray;
		}
		//	doubleArray
		void AddToVarArray_OrginType_doubleArray(double vArrayValue)
		{
			vecdoubleArray.push_back(vArrayValue);
		}
		vector<double>& GetVarArray_OrginType_doubleArray()
		{
			return vecdoubleArray;
		}
		//	shortArray
		void AddToVarArray_OrginType_shortArray(short vArrayValue)
		{
			vecshortArray.push_back(vArrayValue);
		}
		vector<short>& GetVarArray_OrginType_shortArray()
		{
			return vecshortArray;
		}
		//	ushortArray
		void AddToVarArray_OrginType_ushortArray(unsigned short vArrayValue)
		{
			vecushortArray.push_back(vArrayValue);
		}
		vector<unsigned short>& GetVarArray_OrginType_ushortArray()
		{
			return vecushortArray;
		}
		//	charArray
		void AddToVarArray_OrginType_charArray(char vArrayValue)
		{
			veccharArray.push_back(vArrayValue);
		}
		vector<char>& GetVarArray_OrginType_charArray()
		{
			return veccharArray;
		}
		//	ucharArray
		void AddToVarArray_OrginType_ucharArray(unsigned char vArrayValue)
		{
			vecucharArray.push_back(vArrayValue);
		}
		vector<unsigned char>& GetVarArray_OrginType_ucharArray()
		{
			return vecucharArray;
		}
		//	int64Array
		void AddToVarArray_OrginType_int64Array(int64_t vArrayValue)
		{
			vecint64Array.push_back(vArrayValue);
		}
		vector<int64_t>& GetVarArray_OrginType_int64Array()
		{
			return vecint64Array;
		}
		//	uint64Array
		void AddToVarArray_OrginType_uint64Array(uint64_t vArrayValue)
		{
			vecuint64Array.push_back(vArrayValue);
		}
		vector<uint64_t>& GetVarArray_OrginType_uint64Array()
		{
			return vecuint64Array;
		}
	};

	class UserDefinedTypeStruct
	{
	private:
		OrignTypeStruct userDefinedType1;
		OrginTypeArrayFixLenStruct userDefinedType2;
		OrignTypeArrayVarLenStruct userDefindedType3;
	public:
		UserDefinedTypeStruct()
		{
		}
		~UserDefinedTypeStruct()
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
			//	userDefinedType1
			nStructSerializeResult = userDefinedType1.Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen);
			if (nStructSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentSerializeLen += nStructSerializeResult;
			}

			//	userDefinedType2
			nStructSerializeResult = userDefinedType2.Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen);
			if (nStructSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentSerializeLen += nStructSerializeResult;
			}

			//	userDefindedType3
			nStructSerializeResult = userDefindedType3.Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen);
			if (nStructSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentSerializeLen += nStructSerializeResult;
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
			//	userDefinedType1
			nStructUnSerializeResult = userDefinedType1.UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen);
			if (nStructUnSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentUnSerializeLen += nStructUnSerializeResult;
			}

			//	userDefinedType2
			nStructUnSerializeResult = userDefinedType2.UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen);
			if (nStructUnSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentUnSerializeLen += nStructUnSerializeResult;
			}

			//	userDefindedType3
			nStructUnSerializeResult = userDefindedType3.UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen);
			if (nStructUnSerializeResult < 0)
			{
				return -1;
			}
			else
			{
				nCurrentUnSerializeLen += nStructUnSerializeResult;
			}

			return nCurrentUnSerializeLen;
		}
		//	userDefinedType1
		void SetSingleValue_UserDefineType_userDefinedType1(OrignTypeStruct& vValue)
		{
			userDefinedType1 = vValue;
		}
		void GetSingleValue_UserDefineType_userDefinedType1(OrignTypeStruct& vValue)
		{
			vValue = userDefinedType1;
		}
		//	userDefinedType2
		void SetSingleValue_UserDefineType_userDefinedType2(OrginTypeArrayFixLenStruct& vValue)
		{
			userDefinedType2 = vValue;
		}
		void GetSingleValue_UserDefineType_userDefinedType2(OrginTypeArrayFixLenStruct& vValue)
		{
			vValue = userDefinedType2;
		}
		//	userDefindedType3
		void SetSingleValue_UserDefineType_userDefindedType3(OrignTypeArrayVarLenStruct& vValue)
		{
			userDefindedType3 = vValue;
		}
		void GetSingleValue_UserDefineType_userDefindedType3(OrignTypeArrayVarLenStruct& vValue)
		{
			vValue = userDefindedType3;
		}
	};

	class userDefinedTypeFixLenStruct
	{
	private:
		OrignTypeStruct fixLenUserDefinedType1[5];
		OrginTypeArrayFixLenStruct fixLenUserDefinedType2[5];
		OrignTypeArrayVarLenStruct fixLenUserDefinedType3[5];
	public:
		userDefinedTypeFixLenStruct()
		{
		}
		~userDefinedTypeFixLenStruct()
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
			//	fixLenUserDefinedType1
			for (unsigned short usIndex = 0; usIndex < 5; ++usIndex)
			{
				nStructSerializeResult = fixLenUserDefinedType1[usIndex].Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen); 													   
				if (nStructSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					nCurrentSerializeLen += nStructSerializeResult;
				}
			}

			//	fixLenUserDefinedType2
			for (unsigned short usIndex = 0; usIndex < 5; ++usIndex)
			{
				nStructSerializeResult = fixLenUserDefinedType2[usIndex].Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen); 													   
				if (nStructSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					nCurrentSerializeLen += nStructSerializeResult;
				}
			}

			//	fixLenUserDefinedType3
			for (unsigned short usIndex = 0; usIndex < 5; ++usIndex)
			{
				nStructSerializeResult = fixLenUserDefinedType3[usIndex].Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen); 													   
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
			//	fixLenUserDefinedType1
			for (unsigned int unIndex = 0; unIndex < 5; ++unIndex)
			{
				nStructUnSerializeResult = fixLenUserDefinedType1[unIndex].UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen); 													   
				if (nStructUnSerializeResult < 0)
				{
					return -1;
				}else
				{
					nCurrentUnSerializeLen += nStructUnSerializeResult;
				}
			}

			//	fixLenUserDefinedType2
			for (unsigned int unIndex = 0; unIndex < 5; ++unIndex)
			{
				nStructUnSerializeResult = fixLenUserDefinedType2[unIndex].UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen); 													   
				if (nStructUnSerializeResult < 0)
				{
					return -1;
				}else
				{
					nCurrentUnSerializeLen += nStructUnSerializeResult;
				}
			}

			//	fixLenUserDefinedType3
			for (unsigned int unIndex = 0; unIndex < 5; ++unIndex)
			{
				nStructUnSerializeResult = fixLenUserDefinedType3[unIndex].UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen); 													   
				if (nStructUnSerializeResult < 0)
				{
					return -1;
				}else
				{
					nCurrentUnSerializeLen += nStructUnSerializeResult;
				}
			}

			return nCurrentUnSerializeLen;
		}
		//	fixLenUserDefinedType1
		void SetFixArray_UserDefineType_fixLenUserDefinedType1(OrignTypeStruct stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				fixLenUserDefinedType1[i] = stArrayValue[i];
			}
		}
		void GetFixArray_UserDefineType_fixLenUserDefinedType1(OrignTypeStruct stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = fixLenUserDefinedType1[i];
			}
		}
		//	fixLenUserDefinedType2
		void SetFixArray_UserDefineType_fixLenUserDefinedType2(OrginTypeArrayFixLenStruct stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				fixLenUserDefinedType2[i] = stArrayValue[i];
			}
		}
		void GetFixArray_UserDefineType_fixLenUserDefinedType2(OrginTypeArrayFixLenStruct stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = fixLenUserDefinedType2[i];
			}
		}
		//	fixLenUserDefinedType3
		void SetFixArray_UserDefineType_fixLenUserDefinedType3(OrignTypeArrayVarLenStruct stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				fixLenUserDefinedType3[i] = stArrayValue[i];
			}
		}
		void GetFixArray_UserDefineType_fixLenUserDefinedType3(OrignTypeArrayVarLenStruct stArrayValue[5])
		{
			for (int i = 0; i < 5; ++i)
			{
				stArrayValue[i] = fixLenUserDefinedType3[i];
			}
		}
	};

	class userDefinedTypeVarLenStruct
	{
	private:
		vector<OrignTypeStruct> vecvarLenUserDefinedType1;
		vector<OrginTypeArrayFixLenStruct> vecvarLenUserDefinedType2;
		vector<OrignTypeArrayVarLenStruct> vecvarLenUserDefinedType3;
	public:
		userDefinedTypeVarLenStruct()
		{
			vecvarLenUserDefinedType1.clear();
			vecvarLenUserDefinedType2.clear();
			vecvarLenUserDefinedType3.clear();
		}
		~userDefinedTypeVarLenStruct()
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
			//	varLenUserDefinedType1
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_varLenUserDefinedType1 = (unsigned short)vecvarLenUserDefinedType1.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_varLenUserDefinedType1, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_varLenUserDefinedType1; ++usIndex) 													   
			{
				nStructSerializeResult = vecvarLenUserDefinedType1[usIndex].Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen); 													   
				if (nStructSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					nCurrentSerializeLen += nStructSerializeResult;
				}
			}

			//	varLenUserDefinedType2
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_varLenUserDefinedType2 = (unsigned short)vecvarLenUserDefinedType2.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_varLenUserDefinedType2, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_varLenUserDefinedType2; ++usIndex) 													   
			{
				nStructSerializeResult = vecvarLenUserDefinedType2[usIndex].Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen); 													   
				if (nStructSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					nCurrentSerializeLen += nStructSerializeResult;
				}
			}

			//	varLenUserDefinedType3
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen > unBufLen - nCurrentSerializeLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_varLenUserDefinedType3 = (unsigned short)vecvarLenUserDefinedType3.size();
			memcpy(pszBuf + nCurrentSerializeLen, &usDataItemCount_varLenUserDefinedType3, unCurrentDataLen);
			nCurrentSerializeLen += unCurrentDataLen;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_varLenUserDefinedType3; ++usIndex) 													   
			{
				nStructSerializeResult = vecvarLenUserDefinedType3[usIndex].Serialize(pszBuf + nCurrentSerializeLen, unBufLen - nCurrentSerializeLen); 													   
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
			//	varLenUserDefinedType1
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_varLenUserDefinedType1 = 0;
			memcpy(&usDataItemCount_varLenUserDefinedType1, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_varLenUserDefinedType1; ++usIndex) 													   
			{
				OrignTypeStruct classTempData_varLenUserDefinedType1;
				nStructUnSerializeResult = classTempData_varLenUserDefinedType1.UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen); 													   
				if (nStructUnSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					vecvarLenUserDefinedType1.push_back(classTempData_varLenUserDefinedType1);
					nCurrentUnSerializeLen += nStructUnSerializeResult;
				}
			}

			//	varLenUserDefinedType2
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_varLenUserDefinedType2 = 0;
			memcpy(&usDataItemCount_varLenUserDefinedType2, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_varLenUserDefinedType2; ++usIndex) 													   
			{
				OrginTypeArrayFixLenStruct classTempData_varLenUserDefinedType2;
				nStructUnSerializeResult = classTempData_varLenUserDefinedType2.UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen); 													   
				if (nStructUnSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					vecvarLenUserDefinedType2.push_back(classTempData_varLenUserDefinedType2);
					nCurrentUnSerializeLen += nStructUnSerializeResult;
				}
			}

			//	varLenUserDefinedType3
			unCurrentDataLen = sizeof(unsigned short);
			if (unCurrentDataLen + nCurrentUnSerializeLen > unBufLen)
			{
				return -1;
			}
			unsigned short usDataItemCount_varLenUserDefinedType3 = 0;
			memcpy(&usDataItemCount_varLenUserDefinedType3, pszBuf + nCurrentUnSerializeLen, unCurrentDataLen);
			nCurrentUnSerializeLen += unCurrentDataLen;
			for (unsigned short usIndex = 0; usIndex < usDataItemCount_varLenUserDefinedType3; ++usIndex) 													   
			{
				OrignTypeArrayVarLenStruct classTempData_varLenUserDefinedType3;
				nStructUnSerializeResult = classTempData_varLenUserDefinedType3.UnSerialize(pszBuf + nCurrentUnSerializeLen, unBufLen - nCurrentUnSerializeLen); 													   
				if (nStructUnSerializeResult < 0)
				{
					return -1;
				}
				else
				{
					vecvarLenUserDefinedType3.push_back(classTempData_varLenUserDefinedType3);
					nCurrentUnSerializeLen += nStructUnSerializeResult;
				}
			}

			return nCurrentUnSerializeLen;
		}
		//	varLenUserDefinedType1
		void AddToVarArray_UserDefineType_varLenUserDefinedType1(OrignTypeStruct& vArrayValue)
		{
			vecvarLenUserDefinedType1.push_back(vArrayValue);
		}
		vector<OrignTypeStruct>& GetVarArray_UserDefineType_varLenUserDefinedType1()
		{
			return vecvarLenUserDefinedType1;
		}
		//	varLenUserDefinedType2
		void AddToVarArray_UserDefineType_varLenUserDefinedType2(OrginTypeArrayFixLenStruct& vArrayValue)
		{
			vecvarLenUserDefinedType2.push_back(vArrayValue);
		}
		vector<OrginTypeArrayFixLenStruct>& GetVarArray_UserDefineType_varLenUserDefinedType2()
		{
			return vecvarLenUserDefinedType2;
		}
		//	varLenUserDefinedType3
		void AddToVarArray_UserDefineType_varLenUserDefinedType3(OrignTypeArrayVarLenStruct& vArrayValue)
		{
			vecvarLenUserDefinedType3.push_back(vArrayValue);
		}
		vector<OrignTypeArrayVarLenStruct>& GetVarArray_UserDefineType_varLenUserDefinedType3()
		{
			return vecvarLenUserDefinedType3;
		}
	};


}
