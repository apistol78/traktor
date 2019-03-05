#pragma once

#include "Runtime/IRuntimePlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
 */
class T_DLLCLASS RuntimePlugin : public IRuntimePlugin
{
	T_RTTI_CLASS;

public:
	virtual bool create(IEnvironment* environment) override final;

	virtual void destroy(IEnvironment* environment) override final;

	virtual Ref< IState > createInitialState(IEnvironment* environment) override final;
};

	}
}

