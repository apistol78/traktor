#ifndef traktor_Log_H
#define traktor_Log_H

#include <string>
#include <vector>
#include "Core/Io/OutputStream.h"
#include "Core/Io/OutputStreamBuffer.h"
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
	LogTargetConsole(int32_t color);

	virtual void log(const std::wstring& str);
	
private:
	Semaphore m_lock;
	int32_t m_color;
};

/*! \brief Debugger log target.
 * \ingroup Core
 */
class T_DLLCLASS LogTargetDebug : public ILogTarget
{
public:
	virtual void log(const std::wstring& str);

private:
	Semaphore m_lock;
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
	
	virtual int32_t getIndent() const;

	virtual void setIndent(int32_t indent);

	virtual int32_t getDecimals() const;

	virtual void setDecimals(int32_t decimals);

	virtual bool getPushIndent() const;

	virtual void setPushIndent(bool pushIndent);

	virtual int32_t overflow(const wchar_t* buffer, int32_t count);
	
private:
	mutable ThreadLocal m_buffers;
	Ref< ILogTarget > m_target;

	IOutputStreamBuffer* getThreadLocalBuffer() const;
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
