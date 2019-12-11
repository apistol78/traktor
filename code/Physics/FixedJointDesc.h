#pragma once

#include "Physics/JointDesc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! Fixed joint description.
 * \ingroup Physics
 */
class T_DLLCLASS FixedJointDesc : public JointDesc
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;
};

	}
}

