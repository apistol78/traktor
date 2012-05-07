#ifndef traktor_Log_H
#define traktor_Log_H

#include <vector>
#include <string>
#include "Core/Io/IOutputStreamBuffer.h"
#include "Core/Io/OutputStream.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"

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
class T_DLLCLASS ILogTarget : public Object
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
	LogTargetConsole(int color);

	virtual void log(const std::wstring& str);
	
private:
	int m_color;
};

/*! \brief Debugger log target.
 * \ingroup Core
 */
class T_DLLCLASS LogTargetDebug : public ILogTarget
{
public:
	virtual void log(const std::wstring& str);
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
	Semaphore m_lock;
	ThreadLocal m_buffers;
	Ref< ILogTarget > m_target;
};

/*! \brief Log stream.
 * \ingroup Core
 */
class T_DLLCLASS LogStream : public OutputStream
{
public:
	LogStream(ILogTarget* target);

	virtual ~LogStream();

	ILogTarget* getTarget();

	void setTarget(ILogTarget* target);
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

#if defined(_DEBUG)
#	define T_DEBUG(x) traktor::log::debug << x << traktor::Endl
#else
#	define T_DEBUG(x) false
#endif

	}
}

#endif	// traktor_Log_H
