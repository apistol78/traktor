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

/*! \brief Log stream.
 * \ingroup Core
 */
class T_DLLCLASS LogStream : public OutputStream
{
public:
	LogStream(ILogTarget* globalTarget);

	virtual ~LogStream();

	ILogTarget* getGlobalTarget();

	void setGlobalTarget(ILogTarget* target);

	ILogTarget* getLocalTarget();

	void setLocalTarget(ILogTarget* target);
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
