/*
 * LInfoRecorderBase.cpp
 *
 *  Created on: 2015年9月11日
 *      Author: wenshengming
 */

#include "../IncludeHeader.h"
#include "LInfoRecorderBase.h"

LInfoRecorderBase::LInfoRecorderBase()
{
	m_unLogType 	= 0;
	m_unLogLevel 	= 0;
	m_pInfoWriter	= NULL;
}

LInfoRecorderBase::~LInfoRecorderBase()
{

}
void LInfoRecorderBase::SetWriter(LInfoRecorderWriter* pWriter)
{
	m_pInfoWriter = pWriter;
}

void LInfoRecorderBase::SetLogType(int nLogType)
{
	m_unLogType = nLogType;
}
void LInfoRecorderBase::SetLogLevel(int nLevel)
{
	m_unLogLevel = nLevel;
}
int LInfoRecorderBase::GetLogType()
{
	return m_unLogType;
}
int LInfoRecorderBase::GetLogLevel()
{
	return m_unLogLevel;
}

//	unLogType 	日志类型, 按位区别，最大32类
//	unLevel		日志级别，按位区别，最大32级别
//	unLogSubType	日志类型,可能用于区分写入的文件，比如说可能unLogSubType为data时，日志写入unLogType和unLogSubType指定的文件，如果没有设置，那么写入默认文件
//	unThreadID	如果是记录线程的日志时，传送线程ID可以每个线程记录一个文件
//	pszFileName	文件名称
//	pszFunctionName 函数名称
//	nLine				代码第几行
//	pszFmt			日志格式串
//	...					参数
void LInfoRecorderBase::InfoRecorder(unsigned int unLogType, unsigned int unLevel, unsigned int unLogSubType, unsigned int unThreadID,
		const char* pszFileName, const char* pszFunctionName, int nLine,
		const char* pszFmt, va_list ap)
{
	if (!(unLogType & m_unLogType))
	{
		return ;
	}
	if (unLevel < m_unLogLevel)
	{
		return ;
	}
	char szLogContent[MAX_LOG_CONTENT + 1]; memset(szLogContent, 0, sizeof(szLogContent));

	time_t tNow = time(NULL);

	tm* pDateTimeNow = localtime(&tNow);
	int nContentLen = snprintf(szLogContent, MAX_LOG_CONTENT, "<%d-%d-%d %d:%d:%d> [%s][%s][%d]", pDateTimeNow->tm_year + 1900, pDateTimeNow->tm_mon, pDateTimeNow->tm_mday,
			pDateTimeNow->tm_hour, pDateTimeNow->tm_min, pDateTimeNow->tm_sec, pszFileName, pszFunctionName, nLine);
	if (nContentLen < 0)
	{
		return ;
	}

	vsnprintf(szLogContent + nContentLen, MAX_LOG_CONTENT - nContentLen, pszFmt, ap);

	if (m_pInfoWriter != NULL)
	{
		m_pInfoWriter->WriteInfo(unLogType, unLevel, unLogSubType, unThreadID, szLogContent);
	}
}



LInfoRecorderWriter::LInfoRecorderWriter()
{

}

LInfoRecorderWriter::~LInfoRecorderWriter()
{

}

void LInfoRecorderWriter::WriteInfo(unsigned int unLogType, unsigned int unLevel, unsigned int unLobSubType, unsigned int unThreadID, char* pszContent)
{

}
