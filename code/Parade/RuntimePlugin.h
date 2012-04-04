#ifndef traktor_parade_RuntimePlugin_H
#define traktor_parade_RuntimePlugin_H

#include "Amalgam/IRuntimePlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace parade
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

#endif	// traktor_parade_RuntimePlugin_H
