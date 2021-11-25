#pragma once

#include "Core/Class/IRuntimeClassFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! Animation runtime classes factory.
 * \ingroup Animation
 */
class T_DLLCLASS AnimationClassFactory : public IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(IRuntimeClassRegistrar* registrar) const override final;
};

	}
}

