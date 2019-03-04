#pragma once

#include "Amalgam/Game/IRuntimePlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
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

