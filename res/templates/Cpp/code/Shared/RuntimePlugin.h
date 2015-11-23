#ifndef RuntimePlugin_H
#define RuntimePlugin_H

#include <Amalgam/Game/IRuntimePlugin.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MYCPPTEST_SHARED_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class T_DLLCLASS RuntimePlugin : public traktor::amalgam::IRuntimePlugin
{
	T_RTTI_CLASS;

public:
	virtual bool getDependencies(traktor::TypeInfoSet& outDependencies) const T_OVERRIDE T_FINAL;

	virtual bool startup(traktor::amalgam::IEnvironment* environment) T_OVERRIDE T_FINAL;

	virtual bool shutdown(traktor::amalgam::IEnvironment* environment) T_OVERRIDE T_FINAL;

	virtual traktor::Ref< traktor::amalgam::IState > createInitialState(traktor::amalgam::IEnvironment* environment) const T_OVERRIDE T_FINAL;
};

#endif	// RuntimePlugin_H
