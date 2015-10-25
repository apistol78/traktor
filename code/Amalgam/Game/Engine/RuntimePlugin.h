#ifndef traktor_amalgam_RuntimePlugin_H
#define traktor_amalgam_RuntimePlugin_H

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

class T_DLLCLASS RuntimePlugin : public IRuntimePlugin
{
	T_RTTI_CLASS;

public:
	virtual bool getDependencies(TypeInfoSet& outDependencies) const T_OVERRIDE T_FINAL;

	virtual bool startup(IEnvironment* environment) T_OVERRIDE T_FINAL;

	virtual bool shutdown(IEnvironment* environment) T_OVERRIDE T_FINAL;

	virtual Ref< IState > createInitialState(IEnvironment* environment) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_amalgam_RuntimePlugin_H
