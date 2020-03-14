#pragma once

#include "Runtime/IRuntimePlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS RuntimePlugin : public runtime::IRuntimePlugin
{
	T_RTTI_CLASS;

public:
	virtual bool create(runtime::IEnvironment* environment) override final;

	virtual void destroy(runtime::IEnvironment* environment) override final;

	virtual Ref< runtime::IState > createInitialState(runtime::IEnvironment* environment) override final;
};

	}
}

