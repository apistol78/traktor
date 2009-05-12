#ifndef traktor_Log_H
#define traktor_Log_H

#include <vector>
#include <string>
#include <sstream>
#include "Core/Io/OutputStream.h"
#include "Core/Io/OutputStreamBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \brief Log target.
 * \ingroup Core
 */
class T_DLLCLASS LogTarget
{
public:
	virtual ~LogTarget() {}

	virtual void log(const std::wstring& str) = 0;
};

/*! \brief Multiplex log target.
 * \ingroup Core
 */
class T_DLLCLASS LogTargetMultiplex : public LogTarget
{
public:
	LogTargetMultiplex(LogTarget* lt1, LogTarget* lt2 = 0, LogTarget* lt3 = 0, LogTarget* lt4 = 0);
	
	virtual void log(const std::wstring& str);
	
private:
	std::vector< LogTarget* > m_logTargets;
};

/*! \brief Console log target.
 * \ingroup Core
 */
class T_DLLCLASS LogTargetConsole : public LogTarget
{
public:
	LogTargetConsole(const std::wstring& prefix);

	virtual void log(const std::wstring& str);
	
private:
	std::wstring m_prefix;
};

/*! \brief Debugger log target.
 * \ingroup Core
 */
class T_DLLCLASS LogTargetDebug : public LogTarget
{
public:
	LogTargetDebug(const std::wstring& prefix);

	virtual void log(const std::wstring& str);
	
private:
	std::wstring m_prefix;
};

/*! \brief Log stream buffer.
 * \ingroup Core
 */
class T_DLLCLASS LogStreamBuffer : public OutputStreamBuffer
{
public:
	LogStreamBuffer(LogTarget* target = 0);
	
	LogTarget* getTarget();

	void setTarget(LogTarget* target);
	
protected:
	virtual int overflow(const wchar_t* buffer, int count);
	
private:
	LogTarget* m_target;
	std::wstringstream m_ss;
};

/*! \brief Log stream.
 * \ingroup Core
 */
class T_DLLCLASS LogStream : public OutputStream
{
public:
	LogStream(LogTarget* target);

	virtual ~LogStream();

	LogStreamBuffer& getBuffer();

private:
	LogStreamBuffer m_streamBuffer;
};
	
	namespace log
	{

/*! \ingroup Core */
//@{

extern T_DLLCLASS LogStream info;
extern T_DLLCLASS LogStream warning;
extern T_DLLCLASS LogStream error;
extern T_DLLCLASS LogStream debug;

//@}

	}
}

#endif	// traktor_Log_H
