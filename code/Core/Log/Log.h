#pragma once

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

/*! Log target.
 * \ingroup Core
 */
class T_DLLCLASS ILogTarget : public Object
{
public:
	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) = 0;
};

/*! Log stream.
 * \ingroup Core
 */
class T_DLLCLASS LogStream : public OutputStream
{
public:
	explicit LogStream(int32_t level, ILogTarget* globalTarget);

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
#	define T_DEBUG(x)
#endif

	}
}
