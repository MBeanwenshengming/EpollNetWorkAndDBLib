/*
 * LMyErrorRecorder.h
 *
 *  Created on: 2015年10月15日
 *      Author: wenshengming
 */

#ifndef LMYERRORRECORDER_H_
#define LMYERRORRECORDER_H_

#include <IncludeHeader.h>
#include <InfoRecorder/LInfoRecorderBase.h>

class LMyErrorRecorder: public LInfoRecorderWriter
{
public:
	LMyErrorRecorder();
	virtual ~LMyErrorRecorder();
public:
	virtual void WriteInfo(unsigned int unLogType, unsigned int unLevel, unsigned int unLobSubType, unsigned int unThreadID, char* pszContent);

private:
	FILE* m_pErrorFile;
};

#endif /* LMYERRORRECORDER_H_ */
