#pragma once

#include "Amalgam/Game/IRuntimePlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS RuntimePlugin : public amalgam::IRuntimePlugin
{
	T_RTTI_CLASS;

public:
	virtual bool create(amalgam::IEnvironment* environment) override final;

	virtual void destroy(amalgam::IEnvironment* environment) override final;

	virtual Ref< amalgam::IState > createInitialState(amalgam::IEnvironment* environment) override final;
};

	}
}

