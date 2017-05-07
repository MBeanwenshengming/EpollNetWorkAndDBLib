/*
 * LMyErrorRecorder.cpp
 *
 *  Created on: 2015年10月15日
 *      Author: wenshengming
 */

#include "LMyErrorRecorder.h"

LMyErrorRecorder::LMyErrorRecorder()
{
	m_pErrorFile = NULL;
}

LMyErrorRecorder::~LMyErrorRecorder()
{
	if (m_pErrorFile != NULL)
	{
		fclose(m_pErrorFile);

	}
}

void LMyErrorRecorder::WriteInfo(unsigned int unLogType, unsigned int unLevel, unsigned int unLobSubType, unsigned int unThreadID, char* pszContent)
{
	if (m_pErrorFile == NULL)
	{
		m_pErrorFile = fopen("Error.txt", "w+");
		if (m_pErrorFile == NULL)
		{
			return ;
		}
	}
	fwrite(pszContent, strlen(pszContent), 1, m_pErrorFile);
	fflush(m_pErrorFile);
}
