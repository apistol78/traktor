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

class T_DLLCLASS RuntimePlugin : public amalgam::IRuntimePlugin
{
	T_RTTI_CLASS;

public:
	virtual bool getDependencies(TypeInfoSet& outDependencies) const;

	virtual bool startup(amalgam::IEnvironment* environment);

	virtual bool shutdown(amalgam::IEnvironment* environment);

	virtual Ref< amalgam::IState > createInitialState(amalgam::IEnvironment* environment) const;
};

	}
}

#endif	// traktor_amalgam_RuntimePlugin_H
