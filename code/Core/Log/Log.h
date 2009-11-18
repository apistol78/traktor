#ifndef traktor_Log_H
#define traktor_Log_H

#include <vector>
#include <string>
#include <sstream>
#include "Core/Io/IOutputStreamBuffer.h"
#include "Core/Io/OutputStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \brief Log target.
 * \ingroup Core
 */
class ILogTarget
{
public:
	virtual void log(const std::wstring& str) = 0;
};

/*! \brief Console log target.
 * \ingroup Core
 */
class T_DLLCLASS LogTargetConsole : public ILogTarget
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
class T_DLLCLASS LogTargetDebug : public ILogTarget
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
class T_DLLCLASS LogStreamBuffer : public IOutputStreamBuffer
{
public:
	LogStreamBuffer(ILogTarget* target = 0);
	
	ILogTarget* getTarget();

	void setTarget(ILogTarget* target);
	
protected:
	virtual int overflow(const wchar_t* buffer, int count);
	
private:
	ILogTarget* m_target;
	std::wstringstream m_ss;
};

/*! \brief Log stream.
 * \ingroup Core
 */
class T_DLLCLASS LogStream : public OutputStream
{
public:
	LogStream(ILogTarget* target);

	virtual ~LogStream();

	Ref< LogStreamBuffer > getBuffer();

private:
	Ref< LogStreamBuffer > m_streamBuffer;
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
