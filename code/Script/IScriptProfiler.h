/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_IScriptProfiler_H
#define traktor_script_IScriptProfiler_H

#include "Core/Guid.h"
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
		virtual ~IListener() {}

		/*! \brief Called whenever C++ invoke a function into script. */
		virtual void callEnter(const Guid& scriptId, const std::wstring& function) = 0;

		/*! \brief Called after a C++ invoked script function returns. */
		virtual void callLeave(const Guid& scriptId, const std::wstring& function) = 0;

		/*! \brief Called when profiler has measured duration of a script function call. */
		virtual void callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration) = 0;
	};

	virtual void addListener(IListener* listener) = 0;

	virtual void removeListener(IListener* listener) = 0;
};

	}
}

#endif	// traktor_script_IScriptProfiler_H
