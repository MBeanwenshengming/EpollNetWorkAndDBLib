/*
 * LInfoRecorderBase.h
 *
 *  Created on: 2015年9月11日
 *      Author: wenshengming
 */

#ifndef LINFORECORDERBASE_H_
#define LINFORECORDERBASE_H_

#define LOG_TYPE_ACCEPT_THREAD (1 << 0)
#define LOG_TYPE_EPOLL_THREAD (1 << 1)
#define LOG_TYPE_RECV_THREAD (1 << 2)
#define LOG_TYPE_SEND_THREAD (1 << 3)
#define LOG_TYPE_CLOSE_THREAD (1 << 4)
#define LOG_TYPE_MAIN_LOGIC_THREAD (1 << 5)
#define LOG_TYPE_INITIALIZE	(1 << 6)
#define LOG_TYPE_MANAGER		(1 << 7)
#define LOG_TYPE_SESSION		(1 << 8)
#define LOG_TYPE_THREAD_BASE	(1 << 9)
#define LOG_TYPE_NETWORK_SERVICE (1 << 10)
#define LOG_TYPE_CONNECTOR    (1 << 11)


#define LOG_LEVEL_DEBUG 0x01
#define LOG_LEVEL_WARNING 0x02
#define LOG_LEVEL_ERROR 0x04
#define LOG_LEVEL_FATAL 0x08


#define LOG_SUBTYPE_DATA 0x01
#define LOG_SUBTYPE_LOGIC 0x02


#define MAX_LOG_CONTENT 3 * 1024

class LInfoRecorderWriter
{
public:
	LInfoRecorderWriter();
	virtual ~LInfoRecorderWriter();
public:
	virtual void WriteInfo(unsigned int unLogType, unsigned int unLevel, unsigned int unLobSubType, unsigned int unThreadID, char* pszContent);
};

class LInfoRecorderBase
{
public:
	LInfoRecorderBase();
	virtual ~LInfoRecorderBase();
public:
	void SetWriter(LInfoRecorderWriter* pWriter);
	void SetLogType(int nLogType);
	void SetLogLevel(int nLevel);
	int GetLogType();
	int GetLogLevel();

	//	unLogType 	日志类型, 按位区别，最大32类
	//	unLevel		日志级别，按位区别，最大32级别
	//	unLogSubType	日志类型,可能用于区分写入的文件，比如说可能unLogSubType为data时，日志写入unLogType和unLogSubType指定的文件，如果没有设置，那么写入默认文件
	//	unThreadID	如果是记录线程的日志时，传送线程ID可以每个线程记录一个文件，如果不是，那么设置为0
	//	pszFileName	文件名称
	//	pszFunctionName 函数名称
	//	nLine				代码第几行
	//	pszFmt			日志格式串
	//	...					参数
	virtual void InfoRecorder(unsigned int unLogType, unsigned int unLevel, unsigned int unLogSubType, unsigned int unThreadID, const char* pszFileName, const char* pszFunctionName, int nLine, const char* pszFmt, va_list ap);
private:
	unsigned int m_unLogType;
	unsigned int m_unLogLevel;
	LInfoRecorderWriter* m_pInfoWriter;
};



#endif /* LINFORECORDERBASE_H_ */
