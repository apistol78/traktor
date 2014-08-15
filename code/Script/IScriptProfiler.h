#ifndef traktor_script_IScriptProfiler_H
#define traktor_script_IScriptProfiler_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief Script profiler interface.
 * \ingroup Script
 */
class T_DLLCLASS IScriptProfiler : public Object
{
	T_RTTI_CLASS;

public:
	struct IListener
	{
		virtual void callMeasured(const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration) = 0;
	};

	virtual void addListener(IListener* listener) = 0;

	virtual void removeListener(IListener* listener) = 0;
};

	}
}

#endif	// traktor_script_IScriptProfiler_H
