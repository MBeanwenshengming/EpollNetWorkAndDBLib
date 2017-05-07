/*
 * ConnectorThread.h
 *
 *  Created on: 2015年10月27日
 *      Author: wenshengming
 */

#ifndef CONNECTORTHREAD_H_
#define CONNECTORTHREAD_H_
#include "LThreadBase.h"
#include "IncludeHeader.h"
#include "LPacketSingle.h"

class ConnectorThread : public LThreadBase
{
public:
	ConnectorThread();
	virtual ~ConnectorThread();
public:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();
public:
	void CheckIsLittleEndian();
private:
	int m_nSocket;
	bool m_bIsLittleEndian;
};

#endif /* CONNECTORTHREAD_H_ */
